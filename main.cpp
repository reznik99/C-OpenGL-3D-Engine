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

unsigned int g_EntityProgramId = 0, g_TerrainProgramId = 0;
unsigned int shaderIds[4]; //vertexEntities - fragEntities | vertexTerrain - fragTerrain ... etc

std::vector<Entity> entities;
Entity* terrain = nullptr;

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
	const std::string& fragmentShaderSource, unsigned int index) {

	unsigned int vertexShaderId = createShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fragmentShaderId = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	shaderIds[index] = vertexShaderId;
	shaderIds[index + 1] = fragmentShaderId;

	if (vertexShaderId && fragmentShaderId) {
		unsigned int _programId = glCreateProgram();

		glAttachShader(_programId, vertexShaderId);
		glAttachShader(_programId, fragmentShaderId);

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

			glDeleteShader(vertexShaderId);
			glDeleteShader(fragmentShaderId);
			return 0;
		}

		glDetachShader(_programId, vertexShaderId);
		glDetachShader(_programId, fragmentShaderId);

		return _programId;
	}

	return 0;
}

void loadEntity(const char * filename, const char* textureFile, glm::mat4 modelMatrix) {
	Entity newEntity = *readOBJ(filename, textureFile, modelMatrix);

	entities.push_back(newEntity);
}

void init() {
	//Entity shaders and program
	std::string _vertexShaderSource = readShader("shaders/vertexShaderEntities.txt");
	std::string _fragmentShaderSource = readShader("shaders/fragmentShaderEntities.txt");
	g_EntityProgramId = createShaderProgram(_vertexShaderSource, _fragmentShaderSource, 0);
	//Terrain shaders and program
	_vertexShaderSource = readShader("shaders/vertexShaderTerrain.txt");
	_fragmentShaderSource = readShader("shaders/fragmentShaderTerrain.txt");
	g_TerrainProgramId = createShaderProgram(_vertexShaderSource, _fragmentShaderSource, 2);

	//load game Entities
	glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(4, 0, 1));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.3f));
	loadEntity("../Debug/house.obj", "../Debug/house.png", tempModelMatrix);

	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(10, 0, 10));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.3f));
	loadEntity("../Debug/Palm2LowPoly.obj", "../Debug/Palm2.png", tempModelMatrix);

	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(10, 0, 5));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.1f));
	loadEntity("../Debug/well.obj", "../Debug/well3.png", tempModelMatrix);

	//generate terrain
	int mapSize = 50;
	float maxMapHeight = 20;
	terrain = genTerrain(mapSize, maxMapHeight, "../Debug/heightmap.png", glm::translate(glm::mat4(1), glm::vec3(-mapSize/2, maxMapHeight/2, -mapSize / 2)));

	std::cout << "EntProgram ID: " << g_EntityProgramId << std::endl;
	std::cout << "TerProgram ID: " << g_TerrainProgramId << std::endl;
}

void render() {
	glUseProgram(g_EntityProgramId);

	//load uniforms (same for all obj's)
	int projMatrixId = glGetUniformLocation(g_EntityProgramId, "projMatrix");
	int viewMatrixId = glGetUniformLocation(g_EntityProgramId, "viewMatrix");
	int lightPositionId = glGetUniformLocation(g_EntityProgramId, "lightPosition");
	glUniformMatrix4fv(projMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
	glUniform3f(lightPositionId, g_light[0], g_light[1], g_light[2]);

	for (unsigned int i = 0; i < entities.size(); i++) {
		Entity* obj = &entities[i];
		//load uniform for model matrix
		int modelMatrixId = glGetUniformLocation(g_EntityProgramId, "modelMatrix");
		glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &obj->modelMatrix[0][0]);
		//bind texture
		if (obj->textureId > 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, obj->textureId);
		}
		//bind vao
		glBindVertexArray(obj->VAO);
		//render
		glDrawElements(GL_TRIANGLES, obj->indexBufferSize, GL_UNSIGNED_INT, 0);
		//unbind
		glBindVertexArray(0);
		/*glBindBuffer(GL_ARRAY_BUFFER, obj->vertVBOId);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, obj->indexBufferSize);
		glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	}
	
	//RENDER TERRAINS

	glUseProgram(g_TerrainProgramId);

	//load uniforms (same for all obj's)
	projMatrixId = glGetUniformLocation(g_TerrainProgramId, "projMatrix");
	viewMatrixId = glGetUniformLocation(g_TerrainProgramId, "viewMatrix");
	lightPositionId = glGetUniformLocation(g_TerrainProgramId, "lightPosition");
	glUniformMatrix4fv(projMatrixId, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixId, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
	glUniform3f(lightPositionId, g_light[0], g_light[1], g_light[2]);
		
	//load uniform for model matrix
	int modelMatrixId = glGetUniformLocation(g_TerrainProgramId, "modelMatrix");
	glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &terrain->modelMatrix[0][0]);

	//bind texture
	if (terrain->textureId > 0) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrain->textureId);
	}
	//bind vao
	glBindVertexArray(terrain->VAO);
	//render
	glDrawElements(GL_TRIANGLES, terrain->indexBufferSize, GL_UNSIGNED_INT, 0);
	//unbind
	glBindVertexArray(0);

	glUseProgram(0);
}

void display() {
	//animate sun
	float _t = SDL_GetTicks() / 1000.0f; //seconds
	g_light += glm::vec3(sin(_t), 0, cos(_t));
	//g_light = camera.getPosition();

	//clear buffers
	glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
	for(unsigned int _id : shaderIds)
		glDeleteShader(_id);
	glDeleteProgram(g_EntityProgramId);
	glDeleteProgram(g_TerrainProgramId);
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
	/*glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

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

