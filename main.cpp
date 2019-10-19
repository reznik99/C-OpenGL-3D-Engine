#include <iostream>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/GL.h>

#undef main

unsigned int width = 1280, height = 720;

unsigned int g_programId = 0;
unsigned int g_vertexShaderId = 0, g_fragmentShaderId = 0;
unsigned int g_bufferId = 0, g_indexBufferId = 0;


glm::mat4 Projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f) * glm::lookAt(
	glm::vec3(0, 0, 3), // Camera is at (4,3,3), in World Space
	glm::vec3(0, 0, 0), // and looks at the origin
	glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
) * glm::mat4(1.0f);

/*glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f) * glm::lookAt(
	glm::vec3(4, 3, 3), // Camera is at (4,3,3), in World Space
	glm::vec3(0, 0, 0), // and looks at the origin
	glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
) * glm::mat4(1.0f);*/

//should use GLuint for cross plat..
unsigned int createShader(unsigned int type, const std::string& source) {
	unsigned int _id = glCreateShader(type);

	auto _source = source.c_str();
	glShaderSource(_id, 1, &_source, 0);

	glCompileShader(_id);

	int _status = false;
	glGetShaderiv(_id, GL_COMPILE_STATUS, &_status);

	//invalid shader print error
	if (_status == GL_FALSE) {
		int _length = 0;
		glGetShaderiv(_id, GL_INFO_LOG_LENGTH, &_length);

		std::vector<char> _info(_length);
		glGetShaderInfoLog(_id, _length, &_length, _info.data());

		std::cout << std::string(_info.begin(), _info.end()) << std::endl;
		
		glDeleteShader(_id);
		return 0;
	}

	return _id;
}

unsigned int createShaderProgram(const std::string & vertexShaderSource, 
	const std::string& fragmentShaderSource) {

	g_vertexShaderId = createShader(GL_VERTEX_SHADER, vertexShaderSource);
	g_fragmentShaderId = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	if (g_vertexShaderId && g_fragmentShaderId) {
		unsigned int _programId = glCreateProgram();

		glAttachShader(_programId, g_vertexShaderId);
		glAttachShader(_programId, g_fragmentShaderId);

		glLinkProgram(_programId);

		int _status = 0;
		glGetProgramiv(_programId, GL_LINK_STATUS, &_status);

		//couldnt link shaders
		if (_status == GL_FALSE) {
			int _length = 0;
			glGetProgramiv(_programId, GL_INFO_LOG_LENGTH, &_length);

			std::vector<char> _info(_length);
			glGetProgramInfoLog(_programId, _length, &_length, _info.data());

			std::cout << std::string(_info.begin(), _info.end()) << std::endl;

			glDeleteShader(g_vertexShaderId);
			glDeleteShader(g_fragmentShaderId);
			return 0;
		}

		glDetachShader(_programId, g_vertexShaderId);
		glDetachShader(_programId, g_fragmentShaderId);

		return _programId;
	}

	return 0;
}


unsigned int createBuffer() {
	std::vector<float> _data = {
		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,
	};

	unsigned int _bufferId = 0;

	glGenBuffers(1, &_bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _bufferId); //GL_ELEMENT_ARRAY_BUFFER for indices
	glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
	
	glGenBuffers(GL_ARRAY_BUFFER, 0);

	return _bufferId;
}

unsigned int createIndexBuffer() {
	std::vector<unsigned int> _indices = {
		0, 1, 2, 
		2, 1, 3
	};

	unsigned int _bufferId = 0;

	glGenBuffers(1, &_bufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufferId); //GL_ELEMENT_ARRAY_BUFFER for indices
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(float), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW

	glGenBuffers(GL_ELEMENT_ARRAY_BUFFER, 0);

	return _bufferId;
}

void init() {
	std::string _vertexShaderSource = "#version 330 core \n \
										layout (location = 0) in vec3 vertex; \
										uniform mat4 transformationMatrix; \
										void main() { gl_Position = transformationMatrix * vec4(vertex,1.0); }";
	std::string _fragmentShaderSource = "void main() {gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }";

	g_programId = createShaderProgram(_vertexShaderSource, _fragmentShaderSource);
	g_bufferId = createBuffer();
	g_indexBufferId = createIndexBuffer();

	std::cout << "Program ID: " << g_programId << std::endl;
}

void drawTriangle() {
	glUseProgram(g_programId);
	glBindBuffer(GL_ARRAY_BUFFER, g_bufferId);

	//load vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 2nd param is how many dimensions?
	//load uniforms
	int transMatrixLoc = glGetUniformLocation(g_programId, "transformationMatrix");
	glUniformMatrix4fv(transMatrixLoc, 1, GL_FALSE, &Projection[0][0]);


	//glDrawArrays(GL_TRIANGLES, 0, 6); //using vertices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indexBufferId);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);	//using indices

	
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

//animate background color
void display() {
	float _t = SDL_GetTicks() / 1000.0f; //seconds
	float _x = abs(sin(_t))/2;

	Projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f) * glm::lookAt(
		glm::vec3(sin(_t), cos(_t), 2), // Camera is in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	) * glm::mat4(0.5f);

	glClearColor(0.2, 0.4, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	drawTriangle();
}

int main() {

	SDL_Window* _window = SDL_CreateWindow("OpenGL Engine", 
		50, 50, width, height, SDL_WINDOW_OPENGL);

	SDL_GLContext _context = SDL_GL_CreateContext(_window);
	SDL_Event _event;

	glewInit();
	init();

	//main loop
	while (1) {
		bool _break = false;
		//handle events
		while (SDL_PollEvent(&_event)) {
			//quit
			if (_event.type == SDL_KEYDOWN)
				if(_event.key.keysym.sym == SDLK_ESCAPE)
					_break = true;
			if (_event.type == SDL_QUIT)
				_break = true;
			if (_event.type == SDL_MOUSEMOTION)
				//std::cout << "Mouse = (" << _event.motion.x << ", " << _event.motion.y << ")" << std::endl;
			if (_event.button.button == SDL_BUTTON(SDL_BUTTON_RIGHT)) {
				int amount = (_event.motion.x / 100) - 5;
				Projection = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f) * glm::lookAt(
					glm::vec3(0, 0, amount), // Camera is at (4,3,3), in World Space
					glm::vec3(0, 0, 0), // and looks at the origin
					glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
				) * glm::mat4(1.0f);
				std::cout << "Mouse = (" << amount << ", " << _event.motion.y << ")" << std::endl;
			}
		}
		display();

		//sleep
		SDL_Delay(10);
		SDL_GL_SwapWindow(_window);

		if (_break) break;
	}

	//unload buffers
	glDeleteBuffers(1, &g_bufferId);
	glDeleteBuffers(1, &g_indexBufferId);
	//delete shaders
	glDeleteShader(g_vertexShaderId);
	glDeleteShader(g_fragmentShaderId);
	glDeleteProgram(g_programId);
	//close window
	SDL_Quit();
	SDL_DestroyWindow(_window);
	SDL_GL_DeleteContext(_context);

	return 0;
}

