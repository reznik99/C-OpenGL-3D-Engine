#include <iostream>
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <Entity.h>

#undef main

//globals
unsigned int width = 1280, height = 720;
unsigned int g_programId = 0;
unsigned int g_vertexShaderId = 0, g_fragmentShaderId = 0;

std::vector<Entity> entities;

glm::mat4 projectionMatrix = glm::perspective(glm::radians(90.0f), (float)width / (float)height, 0.1f, 100.0f); // 90° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

// Camera matrix
glm::mat4 viewMatrix = glm::lookAt(
	glm::vec3(0, 1, 4), // Camera is in World Space
	glm::vec3(0, 1, 0), // and looks at the origin
	glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
);

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

	Entity newEntity;
	newEntity.load(vertices, indices);

	entities.push_back(newEntity);
}

void init() {
	//should read these from file
	std::string _vertexShaderSource = "#version 330 core \n \
										uniform mat4 projMatrix; \
										uniform mat4 viewMatrix; \
										uniform mat4 modelMatrix; \
										layout (location = 0) in vec3 vertex; \
										out vec3 vertex_pos; \
										void main() { gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(vertex,1.0); vertex_pos = vertex;}";

	std::string _fragmentShaderSource = "in vec3 vertex_pos; \
										void main() {		\
											vec3 vertexCol = (vertex_pos.xyz + 1) / 2; \
											gl_FragColor = vec4(vertexCol, 1.0); }";

	g_programId = createShaderProgram(_vertexShaderSource, _fragmentShaderSource);
	//local url doesn't work for now
	readOBJ("E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/teapot.obj");

	std::cout << "Program ID: " << g_programId << std::endl;
}

void render() {
	glUseProgram(g_programId);

	//load uniforms (same for all obj's)
	int projMatrixId = glGetUniformLocation(g_programId, "projMatrix");
	int viewMatrixId = glGetUniformLocation(g_programId, "viewMatrix");
	glUniformMatrix4fv(projMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &viewMatrix[0][0]);

	for (int i = 0; i < entities.size(); i++) {
		Entity obj = entities[i];
		//load uniforms (same for all obj's)
		int modelMatrixId = glGetUniformLocation(g_programId, "modelMatrix");
		glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &obj.modelMatrix[0][0]);
		//bind vao
		glBindVertexArray(obj.VAO);
		//render
		glDrawElements(GL_TRIANGLES, obj.indexBufferSize, GL_UNSIGNED_INT, 0);
		//unbind
		glBindVertexArray(0);

		obj.modelMatrix = glm::rotate(obj.modelMatrix, 3.14f / 200, glm::vec3(0, 1.0, 0));
	}
	
	glUseProgram(0);
}

void display() {
	//clear buffers
	glClearColor(0.2, 0.4, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	//draw objects
	render();
}

void cleanUp(SDL_Window* _window, SDL_GLContext _context) {
	//unload buffers
	//glDeleteBuffers(1, &g_bufferId);
	//glDeleteBuffers(1, &g_indexBufferId);
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
			viewMatrix = glm::translate(viewMatrix, glm::vec3(0.1, 0, 0));
			break;
		case SDLK_s:
			viewMatrix = glm::translate(viewMatrix, glm::vec3(0, 0, -0.1));
			break;
		case SDLK_d:
			viewMatrix = glm::translate(viewMatrix, glm::vec3(-0.1, 0, 0));
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
		550, 200, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext _context = SDL_GL_CreateContext(_window);
	SDL_Event _event;

	//init
	glewInit();
	init();
	//depth buffer
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//main loop
	while (1) {
		bool _break = false;

		//handle user events
		while (SDL_PollEvent(&_event))
			if (handleUserInput(_event)) _break = true;

		//update entities
		for (Entity e : entities) 
			e.update();

		//display
		display();

		//sleep (should check frametime)
		SDL_Delay(10);
		SDL_GL_SwapWindow(_window);

		if (_break) break;
	}

	cleanUp(_window, _context);

	return 0;
}

