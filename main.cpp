#include <iostream>
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <Entity.h>
#include <Camera.h>
#include "main.h"

#undef main

//globals
unsigned int width = 1280, height = 720;
float FOV = 75.0f;
unsigned int g_programId = 0;
unsigned int g_vertexShaderId = 0, g_fragmentShaderId = 0;

std::vector<Entity> entities;

glm::mat4 projectionMatrix = glm::perspective(glm::radians(FOV), (float)width / (float)height, 0.1f, 100.0f); // 90° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units

Camera camera(glm::vec3(0, 2, 6), glm::vec3(0, 270, -180));

glm::vec3 g_light(25.0f, 100.0f, 0.0f);

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

void readOBJ(const char * filename, glm::mat4 modelMatrix) {
	std::cout << "Reading...:  " << filename << std::endl;
	
	std::vector<unsigned int> indices;
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<unsigned int> normalIndices;

	std::string s;
	std::ifstream fin(filename);
	if (!fin) return;

	while (fin >> s) {
		switch (*s.c_str()) {
		case 'v': {
			if (*(s.c_str() + 1) == 'n') {
				float v1, v2, v3;
				fin >> v1 >> v2 >> v3;
				normals.push_back(v1);
				normals.push_back(v2);
				normals.push_back(v3);
			}else if (*(s.c_str() + 1) == 't') {
				//not implemented
			}
			else {
				float x, y, z;
				fin >> x >> y >> z;
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			}
		}
		break;
		case 'f': {
			unsigned int v1, n1, v2, n2, v3, n3;
			fin >> v1 >> n1 >> v2 >> n2 >> v3 >> n3;
			indices.push_back(v1 - 1);
			indices.push_back(v2 - 1);
			indices.push_back(v3 - 1);
			normalIndices.push_back(n1 - 1);
			normalIndices.push_back(n2 - 1);
			normalIndices.push_back(n3 - 1);
		}
		break;
		}
	}
	int size = normalIndices.size();
	float* orderedNormals = new float[size];

	for (int i = 0; i < size; i++) {
		int currentVertexPointer = indices[i];
		int currentNormalPointer = normalIndices[i];
		orderedNormals[currentVertexPointer * 3] = normals[currentNormalPointer * 3];
		orderedNormals[currentVertexPointer * 3 + 1] = normals[currentNormalPointer * 3 + 1];
		orderedNormals[currentVertexPointer * 3 + 2] = normals[currentNormalPointer * 3 + 2];
	}

	std::vector<float> finalNormals(orderedNormals, orderedNormals + size);

	delete[] orderedNormals;

	std::cout << "Loaded: " << vertices.size() << " " << indices.size() << " " << normals.size() << " " << finalNormals.size() << std::endl;
	Entity newEntity;
	newEntity.load(vertices, indices, finalNormals, &modelMatrix);
	
	entities.push_back(newEntity);
}

void init() {
	//should read these from file
	std::string _vertexShaderSource = "#version 330 core \n \
										uniform mat4 projMatrix; \
										uniform mat4 viewMatrix; \
										uniform mat4 modelMatrix; \
										uniform vec3 lightPosition; \
										in vec3 vertex; \
										in vec3 normal; \
										out vec3 toLightVector; \
										out vec3 surfaceNormal; \
										void main() { \
											vec4 worldPosition = modelMatrix * vec4(vertex, 1.0); \
											gl_Position = projMatrix * viewMatrix * worldPosition; \
											surfaceNormal = (modelMatrix * vec4(normal, 0.0)).xyz; \
											toLightVector = lightPosition - worldPosition.xyz;}";

	std::string _fragmentShaderSource = "in vec3 surfaceNormal; \
										in vec3 toLightVector; \
										void main() {		\
											float nDotl = dot(normalize(surfaceNormal), normalize(toLightVector));\
											float brightness = max(nDotl, 0.2);\
											gl_FragColor = vec4(brightness, brightness, brightness, 1.0); }";

	g_programId = createShaderProgram(_vertexShaderSource, _fragmentShaderSource);

	//local url doesn't work for now
	glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(4, 0, 1));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.3));
	readOBJ("E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/Palm2LowPoly.obj", tempModelMatrix);

	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(10, 0, 10));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.005));
	readOBJ("E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/elepham.obj", tempModelMatrix);

	std::cout << "Program ID: " << g_programId << std::endl;
}

void render() {
	glUseProgram(g_programId);

	//load uniforms (same for all obj's)
	int projMatrixId = glGetUniformLocation(g_programId, "projMatrix");
	int viewMatrixId = glGetUniformLocation(g_programId, "viewMatrix");
	int lightPositionId = glGetUniformLocation(g_programId, "lightPosition");
	glUniformMatrix4fv(projMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
	glUniform3f(lightPositionId, g_light[0], g_light[1], g_light[2]);

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

		//obj.modelMatrix = glm::rotate(obj.modelMatrix, 3.14f / 200, glm::vec3(0, 1.0, 0));
	}
	
	glUseProgram(0);
}

void display() {
	float _t = SDL_GetTicks() / 1000.0f; //seconds
	g_light += glm::vec3(sin(_t), 0, cos(_t));

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
			if (camera.checkInputs(_event)) _break = true;

		//update entities
		for (Entity e : entities) 
			e.update();

		camera.update();

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

