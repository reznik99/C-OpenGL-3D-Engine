
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
#include "UDPClient.h"

#undef main

using namespace std;

extern map<string, vector<unsigned int>> cache;

// Globals
const unsigned int width = 1920, height = 1080;
const int FPS = 60;
const int frameDelay = 1000 / FPS;

Renderer* renderer = nullptr;								// Renderer initialized after main()
Camera camera(glm::vec3(0, 0, 0), glm::vec3(0, 180, 0));	// Players position/view
glm::vec3 g_light;											// Global illumination


void loadEntity(const char* filename, const char* textureFile, const char* textureNormalFile, glm::mat4 modelMatrix) {
	Entity* newEntity = readOBJ_better(filename, textureFile, textureNormalFile, modelMatrix);

	renderer->processEntity(newEntity);
}

void generateEntities(const char* obj, const char* tex, int amount) {
	float terrainSize = renderer->getTerrain()->mapSize;
	for (int i = 0; i < amount; i++) {
		int x = rand() % (int)terrainSize;
		int z = rand() % (int)terrainSize;
		float scale = (rand() % 10) / 10.0f + 1.5f;
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1), glm::vec3(x, renderer->getTerrain()->getHeightAt(z, x), z));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 360)), glm::vec3(0, 1, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(1, 0, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(0, 0, 1));
		loadEntity(obj, tex, NULL, modelMatrix);
	}
}

void init() {
	cout << "Generating Terrain..." << endl;

	renderer = new Renderer(width, height);

	// Generate terrain
	vector<string> terrainTextures{ "gameFiles/Terrain/Blendmap.png","gameFiles/Terrain/Rock.png","gameFiles/Terrain/Grass.png","gameFiles/Terrain/Path.png","gameFiles/Terrain/Sand.png" };
	glm::mat4 terrainModelMatrix = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));

	genTerrain("gameFiles/Terrain/heightmap2_scaled.png", terrainTextures, terrainModelMatrix, renderer->getTerrain(), false);

	// Place player above ground
	float terrainSize = renderer->getTerrain()->mapSize;
	camera.setPosition(glm::vec3(terrainSize / 2, renderer->getTerrain()->getHeightAt((int)(terrainSize / 2), (int)(terrainSize / 2)) + camera.playerHeight, terrainSize / 2));

	// Set light
	g_light = glm::vec3(25.0f, terrainSize / 2, terrainSize * 2); //sunset position (match skybox)

	// Load game Entities
	cout << "Loading game Entities..." << endl;
	
	glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(terrainSize / 3.0f, renderer->getTerrain()->getHeightAt((int)(terrainSize / 3.0f), (int)(terrainSize / 3.0f)), terrainSize / 3.0f));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(3));
	loadEntity("gameFiles/House.obj", "gameFiles/House.png", nullptr, tempModelMatrix);

	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(terrainSize / 2.0f, renderer->getTerrain()->getHeightAt(terrainSize / 2.0f, terrainSize / 3.0f), terrainSize / 3.0f));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(3));
	loadEntity("gameFiles/moonbrook_inn.obj", "gameFiles/moonbrook_inn.png", nullptr, tempModelMatrix);


	tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(275, renderer->getTerrain()->getHeightAt(275, 425) - 1, 425));
	tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(8));
	loadEntity("gameFiles/Tower.obj", "gameFiles/Tower.png", nullptr, tempModelMatrix);

	
	tempModelMatrix = glm::translate(glm::mat4(1), camera.getPosition() - glm::vec3(0, camera.playerHeight, 0));
	readOBJ_better("gameFiles/Stone.obj", "gameFiles/Stone.png", nullptr, tempModelMatrix); //cache player model

	// Random entities
	generateEntities("gameFiles/Palm.obj", "gameFiles/Palm.png", 2500);
	generateEntities("gameFiles/Palm2_low.obj", "gameFiles/Palm2.jpg", 2500);
	generateEntities("gameFiles/grass.obj", "gameFiles/grass.png", 30000);
}



void cleanUp(SDL_Window* _window, SDL_GLContext _context, UDPClient* client) {
	if (client->connectedStatus)
		client->cleanUp();
	renderer->cleanUp();
	// Close window
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
	cout << "Enter server ip (ip/domain:port) ";
	string url;
	getline(cin, url);
	string port = url.substr(url.find(":") + 1, url.size());

	//TCPClient* client = new TCPClient(url.substr(0, url.find(":")), port);
	UDPClient* client = new UDPClient(url.substr(0, url.find(":")), port);
	future<void> tcpPromise;


	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12); // Terminal color

	init(); // Load game data

	SetConsoleTextAttribute(hConsole, 2);
	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

	SetConsoleTextAttribute(hConsole, 7);

	// Framerate control
	Uint32 frameStart;
	int frameTime;
	bool firstLoop = true;
	
	// Game loop
	while (1) {
		bool _break = false;
		frameStart = SDL_GetTicks();

		// Handle user events
		while (SDL_PollEvent(&_event))
			if (camera.checkInputs(_event)) _break = true;

		// Update
		if (client->connectedStatus) {
			if (firstLoop || tcpPromise.wait_for(chrono::milliseconds(0)) == future_status::ready) {
				tcpPromise = async(launch::async, &UDPClient::update, client, camera.getPosition(), camera.getAngles().y, renderer);
				firstLoop = false;
			}
		}

		renderer->update();
		camera.update(renderer->getTerrain());

		// Render
		renderer->render(g_light, camera);

		// Display
		SDL_GL_SwapWindow(_window);

		// Sleep
		frameTime = SDL_GetTicks() - frameStart;
		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);

		if (_break) break;
	}

	cleanUp(_window, _context, client);

	return 0;
}

