#pragma warning(disable:4996)
#pragma once
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
#include <STB/stb_image.h>
#include "Loader.h"

#undef main


//globals
const unsigned int width = 1280, height = 720;
const float FOV = 75.0f;
const int FPS = 60;
const int frameDelay = 1000 / FPS;

unsigned int g_programId = 0, g_vertexShaderId = 0, g_fragmentShaderId = 0;

std::vector<Entity> entities;

// 90° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
glm::mat4 projectionMatrix = glm::perspective(glm::radians(FOV), (float)width / (float)height, 0.1f, 100.0f);
Camera camera(glm::vec3(0, 2, 6), glm::vec3(0, 270, -180));
glm::vec3 g_light(25.0f, 20.0f, 0.0f);

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

void loadEntity(const char * filename, const char* textureFile, glm::mat4 modelMatrix) {
	Entity newEntity = *readOBJ(filename, textureFile, modelMatrix);

	entities.push_back(newEntity);
}

void init() {
	
	std::string _vertexShaderSource = readShader("shaders/vertexShaderEntities.txt");
	std::string _fragmentShaderSource = readShader("shaders/fragmentShaderEntities.txt");

	g_programId = createShaderProgram(_vertexShaderSource, _fragmentShaderSource);

	
	//load game Entities
	glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(4, 0, 1));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.3f));
	loadEntity("../Debug/house.obj", "../Debug/house.png", tempModelMatrix);

	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(10, 0, 10));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.3f));
	//loadEntity("E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/Palm2LowPoly.obj", "E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/Palm2.png", tempModelMatrix);

	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(10, 0, 5));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.1f));
	//loadEntity("E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/well.obj", "E:/UNI/ExtraCurricular/OpenGL/C++/C++OpenGL_1/Debug/well3.png", tempModelMatrix);

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

	for (unsigned int i = 0; i < entities.size(); i++) {
		Entity* obj = &entities[i];
		//load uniform for model matrix
		int modelMatrixId = glGetUniformLocation(g_programId, "modelMatrix");
		glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &obj->modelMatrix[0][0]);
		//bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, obj->textureId);
		//bind vao
		glBindVertexArray(obj->VAO);
		//render
		glDrawElements(GL_TRIANGLES, obj->indexBufferSize, GL_UNSIGNED_INT, 0);
		//unbind
		glBindVertexArray(0);
	}
	
	glUseProgram(0);
}

void display() {
	//animate sun
	float _t = SDL_GetTicks() / 1000.0f; //seconds
	g_light += glm::vec3(sin(_t), 0, cos(_t));

	//clear buffers
	glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	//draw objects
	render();
}

void cleanUp(SDL_Window* _window, SDL_GLContext _context) {
	//unload entities
	for (Entity e : entities) {
		//this should be handled by a loader class that only loads obj's if unique (not implemented yet)
		//delete VBOs
		glDeleteBuffers(1, &e.vertVBOId);
		glDeleteBuffers(1, &e.normVBOId);
		glDeleteBuffers(1, &e.texVBOId);
		//delete VAO
		glDeleteVertexArrays(1, &e.VAO);
	}
	entities.clear(); //calls deconstructor in all entities
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

	//framerate control
	Uint32 frameStart;
	int frameTime;

	//main loop
	while (1) {
		bool _break = false;

		frameStart = SDL_GetTicks();

		//handle user events
		while (SDL_PollEvent(&_event))
			if (camera.checkInputs(_event)) _break = true;

		//update entities
		for (Entity e : entities) 
			e.update();

		camera.update();

		//display
		display();
		SDL_GL_SwapWindow(_window);

		frameTime = SDL_GetTicks() - frameStart;
		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);

		if (_break) break;
	}

	cleanUp(_window, _context);

	return 0;
}

