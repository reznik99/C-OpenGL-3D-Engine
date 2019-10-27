#pragma warning(disable:4996)
#pragma once
#include <iostream>
#include <vector>
#include <fstream>

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <Entity.h>
#include <Camera.h>
#include <STB/stb_image.h>
#include "Loader.h"
#include <Renderer.h>

#undef main

//globals
const unsigned int width = 1280, height = 720;
const int FPS = 60;
const int frameDelay = 1000 / FPS;

Renderer* renderer = nullptr;
Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
glm::vec3 g_light;


void loadEntity(const char * filename, const char* textureFile, glm::mat4 modelMatrix) {
	Entity newEntity = *readOBJ_better(filename, textureFile, modelMatrix);

	renderer->processEntity(newEntity);
}

void init() {
	renderer = new Renderer(width, height);

	//generate terrain
	glm::mat4 terrainModelMatrix = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
	genTerrain("gameFiles/Heightmap.png", "gameFiles/Rock.png", terrainModelMatrix, renderer->getTerrain());

	//place player above ground
	float terrainSize = renderer->getTerrain()->mapSize;
	camera.setPosition(glm::vec3(terrainSize / 2, renderer->getTerrain()->getHeightAt(terrainSize / 2, terrainSize / 2) + camera.playerHeight, terrainSize / 2));

	//set light
	g_light = glm::vec3(terrainSize * 2, terrainSize, 25.0f);

	//load game Entities
	{
		glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(45, renderer->getTerrain()->getHeightAt(35, 45), 35));
		tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.35f));
		loadEntity("gameFiles/House.obj", "gameFiles/House.png", tempModelMatrix);

		tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(40, renderer->getTerrain()->getHeightAt(28, 40), 28));
		tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.15f));
		loadEntity("gameFiles/Well.obj", "gameFiles/Well.png", tempModelMatrix);

		int numOfTrees = 10;
		for (int i = 0; i < numOfTrees; i++) {
			int x = rand() % (int)terrainSize + 1;
			int z = rand() % (int)terrainSize + 1;
			float scale = (rand() % 10) / 10.0f + 0.15f;
			tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(x, renderer->getTerrain()->getHeightAt(z, x), z));
			tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(scale));
			loadEntity("gameFiles/Palm.obj", "gameFiles/Palm.png", tempModelMatrix);
		}
	}
}

void cleanUp(SDL_Window* _window, SDL_GLContext _context) {
	renderer->cleanUp();
	//close window
	SDL_Quit();
	SDL_DestroyWindow(_window);
	SDL_GL_DeleteContext(_context);
}

int main() {
	//set up window
	SDL_Window* _window = SDL_CreateWindow("OpenGL Engine", 
		600, 50, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext _context = SDL_GL_CreateContext(_window);
	SDL_Event _event;

	//init
	glewInit();
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	init();

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

		//update
		renderer->update();
		camera.update(renderer);

		//render
		renderer->render(g_light, camera);

		//display
		SDL_GL_SwapWindow(_window);

		frameTime = SDL_GetTicks() - frameStart;
		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);

		if (_break) break;
	}

	cleanUp(_window, _context);

	return 0;
}

