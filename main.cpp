#include <iostream>
#include <vector>
#include <fstream>

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/GL.h>

#undef main

unsigned int width = 1280, height = 720;

unsigned int g_programId = 0;
unsigned int g_vertexShaderId = 0, g_fragmentShaderId = 0;
//these should be stored in entity class (per object basis)
unsigned int g_bufferId = 0, g_indexBufferId = 0;
unsigned int g_vBufferSize = 0, g_vIndexBufferSize = 0;

// Projection matrix : 90° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f);

//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates for an ortho camera :

// Camera matrix
glm::mat4 viewMatrix = glm::lookAt(
	glm::vec3(0, 1, 4), // Camera is in World Space
	glm::vec3(0, 1, 0), // and looks at the origin
	glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
);
// Model matrix : an identity matrix (model will be at the origin)
glm::mat4 modelMatrix = glm::mat4(1.0f);

// ModelViewProjection : multiplication of our 3 matrices
glm::mat4 mvp = projectionMatrix * viewMatrix * modelMatrix; // Remember, matrix multiplication is the other way around

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


unsigned int createBuffer(std::vector<float> &_data) {
	unsigned int _bufferId = 0;

	glGenBuffers(1, &_bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _bufferId);
	glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
	
	glGenBuffers(GL_ARRAY_BUFFER, 0);

	return _bufferId;
}

unsigned int createIndexBuffer(std::vector<unsigned int> &_indices) {
	unsigned int _bufferId = 0;

	glGenBuffers(1, &_bufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _bufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW

	glGenBuffers(GL_ELEMENT_ARRAY_BUFFER, 0);

	return _bufferId;
}

void readOBJ(const char * filename) {
	std::cout << "Reading...:  " << filename << std::endl;
	
	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	std::string s;
	std::ifstream fin(filename);
	if (!fin) return;

	while (fin >> s) {
		switch (*s.c_str()) {
		case 'v': {
			float x, y, z;
			fin >> x >> y >> z;
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
		break;
		case 'f': {
			int v1, v2, v3;
			fin >> v1 >> v2 >> v3;
			indices.push_back(v1 - 1);
			indices.push_back(v2 - 1);
			indices.push_back(v3 - 1);
		}
		break;
		}
	}
	//create buffers with data
	g_bufferId = createBuffer(vertices);
	g_indexBufferId = createIndexBuffer(indices);

	//store their sizes for glDrawElements call
	g_vBufferSize = vertices.size();
	g_vIndexBufferSize = indices.size();

	std::cout << "Read " << vertices.size() / 3 << " vertices" << std::endl;
	std::cout << "Read " << indices.size() / 3 << " faces" << std::endl;
}

void init() {
	//should read these from file
	std::string _vertexShaderSource = "#version 330 core \n \
										layout (location = 0) in vec3 vertex; \
										uniform mat4 mvpMatrix; \
										out vec3 vertex_pos; \
										void main() { gl_Position = mvpMatrix * vec4(vertex,1.0); vertex_pos = vertex;}";

	std::string _fragmentShaderSource = "in vec3 vertex_pos; \
										void main() {		\
											vec3 vertexCol = (vertex_pos.xyz + 1)/2; \
											gl_FragColor = vec4(vertexCol, 1.0); }";

	g_programId = createShaderProgram(_vertexShaderSource, _fragmentShaderSource);
	//local url doesn't work for now
	readOBJ("E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/teapot.obj");

	std::cout << "Program ID: " << g_programId << std::endl;
}

void drawObject() {
	glUseProgram(g_programId);
	glBindBuffer(GL_ARRAY_BUFFER, g_bufferId);

	//load vertices
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // 2nd param is how many dimensions?
	//load uniforms
	int transMatrixLoc = glGetUniformLocation(g_programId, "mvpMatrix");
	glUniformMatrix4fv(transMatrixLoc, 1, GL_FALSE, &mvp[0][0]);

	//glDrawArrays(GL_TRIANGLES, 0, 6); //using vertices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indexBufferId);
	glDrawElements(GL_TRIANGLES, g_vIndexBufferSize, GL_UNSIGNED_INT, 0);	//using indices
	
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glUseProgram(0);
}

void display() {
	//update matrices
	mvp = projectionMatrix * viewMatrix * modelMatrix;

	//clear buffers
	glClearColor(0.2, 0.4, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	//draw objects
	drawObject();
}

void cleanUp(SDL_Window* _window, SDL_GLContext _context) {
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
}

bool handleUserInput(SDL_Event &_event) {
	if (_event.type == SDL_QUIT)
		return true;
	if (_event.type == SDL_KEYDOWN) {
		switch (_event.key.keysym.sym) {
		case SDLK_w:
			viewMatrix = glm::translate(viewMatrix, glm::vec3(0, 0, 0.1));
			break;
		case SDLK_a:
			viewMatrix = glm::translate(viewMatrix, glm::vec3(0, 0, 0.1));
			break;
		case SDLK_s:
			viewMatrix = glm::translate(viewMatrix, glm::vec3(0, 0, -0.1));
			break;
		case SDLK_d:
			viewMatrix = glm::translate(viewMatrix, glm::vec3(0, 0, 0.1));
			break;
		case SDLK_ESCAPE:
			return true;
			break;
		}
	}
	return false;
}

int main() {
	//set up window
	SDL_Window* _window = SDL_CreateWindow("OpenGL Engine", 
		50, 50, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext _context = SDL_GL_CreateContext(_window);
	SDL_Event _event;

	//init
	glewInit();
	init();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//main loop
	while (1) {
		bool _break = false;

		//handle user events
		while (SDL_PollEvent(&_event)) {
			if (handleUserInput(_event)) _break = true;
		}

		display();

		//animate object (rotate it)
		modelMatrix = glm::rotate(modelMatrix, 3.14f / 200, glm::vec3(0, 1.0, 0)); // where x, y, z is axis of rotation (e.g. 0 1 0)

		//sleep (should check frametime)
		SDL_Delay(10);
		SDL_GL_SwapWindow(_window);

		if (_break) break;
	}

	cleanUp(_window, _context);

	return 0;
}

