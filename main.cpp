#pragma warning(disable:4996)
#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <future>
#include <chrono>

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <STB/stb_image.h>

#include "Entity.h"
#include "Camera.h"
#include "Loader.h"
#include "Renderer.h"
#include "TCPClient.h"

#undef main

extern map<string, vector<unsigned int>> cache;

//globals
bool online = false;
const unsigned int width = 1080, height = 720;
const int FPS = 60;
const int frameDelay = 1000 / FPS;

Renderer* renderer = nullptr;
Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));
glm::vec3 g_light;


void loadEntity(const char * filename, const char* textureFile, const char* textureNormalFile, glm::mat4 modelMatrix) {
	Entity newEntity = *readOBJ_better(filename, textureFile, textureNormalFile, modelMatrix);

	renderer->processEntity(newEntity);
}

void init() {
	renderer = new Renderer(width, height);

	//generate terrain
	std::cout << "Generating Terrain.." << std::endl;
	std::vector<std::string> terrainTextures{ "gameFiles/Terrain/Blendmap.png", "gameFiles/Terrain/Rock.png", 
		"gameFiles/Terrain/Grass.png", "gameFiles/Terrain/Path.png", "gameFiles/Terrain/Sand.png" };
	glm::mat4 terrainModelMatrix = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
	genTerrain("gameFiles/Terrain/heightmap2_scaled.png", terrainTextures, terrainModelMatrix, renderer->getTerrain());

	//place player above ground
	float terrainSize = renderer->getTerrain()->mapSize;
	camera.setPosition(glm::vec3(terrainSize / 2.0f, renderer->getTerrain()->getHeightAt((int)(terrainSize / 2), (int)(terrainSize / 2)) + camera.playerHeight, (int)(terrainSize / 2)));

	//set light
	g_light = glm::vec3(25.0f, terrainSize, terrainSize * 2);

	//load game Entities
	std::cout << "Loading game Entities..." << std::endl;
	{
		//hard coded entities
		glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3((int)(terrainSize/3), renderer->getTerrain()->getHeightAt((int)(terrainSize/3), (int)(terrainSize/3)), (int)(terrainSize/3)));
		tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.5f));
		loadEntity("gameFiles/House.obj", "gameFiles/House.png", nullptr, tempModelMatrix);

		tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3((int)(terrainSize/3) + 10, renderer->getTerrain()->getHeightAt((int)(terrainSize/3) + 10, (int)(terrainSize/3) +10), terrainSize/3 + 10));
		tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.15f));
		loadEntity("gameFiles/Well.obj", "gameFiles/Well.png", nullptr, tempModelMatrix);

		tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3((int)(terrainSize/3) + 12, renderer->getTerrain()->getHeightAt((int)(terrainSize/3) + 12, (int)(terrainSize/3) + 12), (int)(terrainSize/3) + 12));
		tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.5f));
		loadEntity("gameFiles/grass.obj", "gameFiles/grass.png", nullptr, tempModelMatrix);

		//load player model
		tempModelMatrix = glm::mat4(1);
		Entity* player = readOBJ_better("gameFiles/Player.obj", "gameFiles/Well.png", nullptr, tempModelMatrix);


		//random entities
		int numOfTrees = 200;
		for (int i = 0; i < numOfTrees; i++) {
			int x = rand() % (int)terrainSize;
			int z = rand() % (int)terrainSize;
			float scale = (rand() % 10) / 10.0f + 0.15f;
			tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(x, renderer->getTerrain()->getHeightAt(z, x), z));
			tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(scale));
			tempModelMatrix = glm::rotate(tempModelMatrix, glm::radians((float)(rand() %  360)), glm::vec3(0, 1, 0));
			tempModelMatrix = glm::rotate(tempModelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(1, 0, 0));
			tempModelMatrix = glm::rotate(tempModelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(0, 0, 1));
			if(i < numOfTrees/2) loadEntity("gameFiles/Palm2_low.obj", "gameFiles/Palm2.jpg", nullptr, tempModelMatrix);
			else loadEntity("gameFiles/Palm.obj", "gameFiles/Palm.png", nullptr, tempModelMatrix);
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

	//networking
	TCPClient* client = NULL;
	std::future<void> tcpPromise;
	bool firstLoop = true;
	if (online) {
		client = new TCPClient("localhost", "8080");
		if (!client->connectedStatus)
			online = false;
	}
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12);

	init(); //load game data
	SetConsoleTextAttribute(hConsole, 2);

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	SetConsoleTextAttribute(hConsole, 7);

	//framerate control
	Uint32 frameStart;
	int frameTime;

	vector<unsigned int> ids = cache.at("gameFiles/Player.obj");
	
	//main loop
	while (1) {
		bool _break = false;
		frameStart = SDL_GetTicks();

		//handle user events
		while (SDL_PollEvent(&_event))
			if (camera.checkInputs(_event)) _break = true;

		//update
		if (online) { //players
			if (firstLoop || tcpPromise.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				tcpPromise = std::async(std::launch::async, &TCPClient::update, client, camera.getPosition(), camera.getAngles().y, renderer, ids);
				firstLoop = false;
			}
		}

		renderer->update();
		camera.update(renderer->getTerrain());

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

	if(online)
		client->cleanUp();

	return 0;
}

