
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
const unsigned int width = 1920, height = 1080;
const int FPS = 60;
const int frameDelay = 1000 / FPS;
bool online = true;

Renderer* renderer = nullptr; //renderer initialized after main()
Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0));	//players position/view
glm::vec3 g_light;  //global illumination


void loadEntity(const char* filename, const char* textureFile, const char* textureNormalFile, glm::mat4 modelMatrix) {
	Entity newEntity = *readOBJ_better(filename, textureFile, textureNormalFile, modelMatrix);

	renderer->processEntity(newEntity);
}

void generateEntities(const char* obj, const char* tex, int amount) {
	float terrainSize = renderer->getTerrain()->mapSize;
	for (int i = 0; i < amount; i++) {
		int x = rand() % (int)terrainSize;
		int z = rand() % (int)terrainSize;
		float scale = (rand() % 10) / 10.0f + 0.15f;
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1), glm::vec3(x, renderer->getTerrain()->getHeightAt(z, x), z));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 360)), glm::vec3(0, 1, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(1, 0, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(0, 0, 1));
		loadEntity(obj, tex, NULL, modelMatrix);
	}
}

void init() {
	cout << "Generating Terrain.." << std::endl;

	renderer = new Renderer(width, height);

	//generate terrain
	vector<string> terrainTextures{ "gameFiles/Terrain/Blendmap.png","gameFiles/Terrain/Rock.png","gameFiles/Terrain/Grass.png","gameFiles/Terrain/Path.png","gameFiles/Terrain/Sand.png" };
	glm::mat4 terrainModelMatrix = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));

	genTerrain("gameFiles/Terrain/heightmap2_scaled.png", terrainTextures, terrainModelMatrix, renderer->getTerrain(), true);

	//place player above ground
	float terrainSize = renderer->getTerrain()->mapSize;
	camera.setPosition(glm::vec3(terrainSize / 2, renderer->getTerrain()->getHeightAt((int)(terrainSize / 2), (int)(terrainSize / 2)) + camera.playerHeight, terrainSize / 2));

	//set light
	g_light = glm::vec3(25.0f, terrainSize / 2, terrainSize * 2); //sunset position (match skybox)

	// load game Entities
	std::cout << "Loading game Entities..." << std::endl;
	
	glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(terrainSize / 3.0f, renderer->getTerrain()->getHeightAt((int)(terrainSize / 3.0f), (int)(terrainSize / 3.0f)), terrainSize / 3.0f));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(0.7f));
	loadEntity("gameFiles/House.obj", "gameFiles/House.png", nullptr, tempModelMatrix);

	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(50, renderer->getTerrain()->getHeightAt(50, 50)+1, 50));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(1));
	loadEntity("gameFiles/moonbrook_inn.obj", "gameFiles/moonbrook_inn.png", nullptr, tempModelMatrix);
	
	tempModelMatrix = glm::translate(glm::mat4(1), camera.getPosition());
	readOBJ_better("gameFiles/Player.obj", "gameFiles/Well.png", nullptr, tempModelMatrix); //cache player model

	/*tempModelMatrix = glm::translate(glm::mat4(1), camera.getPosition());
	string playerId = "test";
	Entity* player = readOBJ_better("gameFiles/Player.obj", "gameFiles/Well.png", nullptr, tempModelMatrix);
	pair<string, Entity*> p = pair<string, Entity*>(playerId, player);
	renderer->players.insert(p);*/

	//random entities
	generateEntities("gameFiles/Palm.obj", "gameFiles/Palm.png", 150);
	generateEntities("gameFiles/Palm2_low.obj", "gameFiles/Palm2.jpg", 150);
	generateEntities("gameFiles/grass.obj", "gameFiles/grass.png", 400);
}



void cleanUp(SDL_Window* _window, SDL_GLContext _context) {
	renderer->cleanUp();
	//close window
	SDL_Quit();
	SDL_DestroyWindow(_window);
	SDL_GL_DeleteContext(_context);
}


int main(int argc, char* argv[]) {
	// SDL
	SDL_Window* _window = SDL_CreateWindow("OpenGL Engine",
		600, 50, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext _context = SDL_GL_CreateContext(_window);
	SDL_Event _event;

	// Init
	glewInit();
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// Networking
	TCPClient* client = new TCPClient("localhost", "8080");
	std::future<void> tcpPromise;
	if (!client->connectedStatus) //if can't connect, play offline
		online = false;


	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12); //terminal color

	init(); //load game data

	SetConsoleTextAttribute(hConsole, 2);
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	SetConsoleTextAttribute(hConsole, 7);

	//framerate control
	Uint32 frameStart;
	int frameTime;
	bool firstLoop = true;
	//vector<unsigned int> ids = cache.at("gameFiles/Player.obj");
	
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
				tcpPromise = std::async(std::launch::async, &TCPClient::update, client, camera.getPosition(), camera.getAngles().y, renderer);
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

	if (online)
		client->cleanUp();

	return 0;
}

