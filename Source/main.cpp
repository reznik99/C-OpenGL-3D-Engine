
// STL
#include <iostream>
#include <vector>
#include <fstream>
#include <future>
#include <chrono>

//Math
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

// OpenGL extensios
#include <GL/glew.h>

// Image loading
#include <STB/stb_image.h>

// Engine
#include "../Headers/model.h"
#include "../Headers/camera.h"
#include "../Headers/loader.h"
#include "../Headers/entity.h"
#include "../Headers/skybox.h"
#include "../Headers/UDPClient.h"

#undef main

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
const int FPS = 60;
const int frameDelay = 1000 / FPS;

// Game
vector<Entity> entities;
map<string, Entity*> players;
string playerName;
Model* playerModel;

// Camera
Camera camera(glm::vec3(200, 100, 200), glm::vec3(0, 180, 0));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Generates n entities randomly on the map
void generateEntities(Model* model, Terrain* terrain, int amount, float minScale) {
	float terrainSize = terrain->mapSize;
	for (int i = 0; i < amount; i++) {
		int x = rand() % (int)terrainSize;
		int z = rand() % (int)terrainSize;
		float scale = (rand() % 10) / 5.0f + minScale;
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1), glm::vec3(x, terrain->getHeightAt(z, x) - 3.0f, z));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 360)), glm::vec3(0, 1, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(1, 0, 0));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((float)(rand() % 25)), glm::vec3(0, 0, 1));
		entities.push_back(Entity(model, modelMatrix));
	}
}

int main(int argc, char** argv)
{
	// SDL
	SDL_Window* _window = SDL_CreateWindow("OpenGL Engine",
		600, 50, SCR_WIDTH, SCR_HEIGHT, SDL_WINDOW_OPENGL);
	SDL_GLContext _context = SDL_GL_CreateContext(_window);
	SDL_Event _event;

	// Init
	glewInit();
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);

	//stbi_set_flip_vertically_on_load(true);

	// Depth test and msaa
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);

	// alpha
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Build and compile shaders
	printf("Loading Shaders...");
	Shader SkyboxShader("./Engine/Shaders/Skybox-Vertex.glsl", "./Engine/Shaders/Skybox-Fragment.glsl");
	Shader TerrainShader("./Engine/Shaders/Terrain-Vertex.glsl", "./Engine/Shaders/Terrain-Fragment.glsl");
	Shader EntityShader("./Engine/Shaders/Entity-Vertex.glsl", "./Engine/Shaders/Entity-Fragment.glsl");
	printf("Done.\n");

	// Generate terrain
	printf("Generating World...");
	string terrainPath = "./Engine/Models/Terrain/";

	vector<string> terrainTextures{ terrainPath + "Blendmap.png",terrainPath + "Grass.png",
		terrainPath + "Grass2.png", terrainPath + "Sand.png", terrainPath + "Rock.png" };
	glm::mat4 terrainModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

	Terrain terrain;
	genTerrain((terrainPath + "smol_heightmap.png").c_str(), terrainTextures, terrainModelMatrix, &terrain, false);
	// Set Global illumination
	glm::vec3 g_light = glm::vec3(250.0f, 1000.0f, 250.0f); //sunset position (match skybox)

	// Load Skybox
	string skyboxPath = "./Engine/Models/Skybox/Day/";
	vector<string> files = { skyboxPath + "right.png", skyboxPath + "left.png", skyboxPath + "top.png",
			skyboxPath + "bottom.png", skyboxPath + "back.png", skyboxPath + "front.png" };
	Skybox skybox(files);
	printf("Done.\n");

	// Load models
	printf("Loading Models...");
	Model gs_inn = Model("./Engine/Models/Goldshirehouse/goldshireinn.obj");
	Model tree1 = Model("./Engine/Models/Tree/Tree.obj");
	Model tree2 = Model("./Engine/Models/Palm/10446_Palm_Tree_v1_max2010_iteration-2-small.obj");
	Model grass = Model("./Engine/Models/Grass/grass.obj");
	playerModel = new Model("./Engine/Models/Orc/orcmalescale.obj");
	printf("Done.\n");

	// Placing Entities
	printf("Loading map...");
	glm::mat4 gs_inn_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(200, terrain.getHeightAt(200, 100) - 4.0f, 100));
	gs_inn_matrix = glm::scale(gs_inn_matrix, glm::vec3(3.5f, 3.5f, 3.5f));
	gs_inn_matrix = glm::rotate(gs_inn_matrix, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	entities.push_back(Entity(&gs_inn, gs_inn_matrix));

	
	glm::mat4 npc1_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(250, terrain.getHeightAt(250, 250) - 2.0f, 250));
	npc1_matrix = glm::scale(npc1_matrix, glm::vec3(7.0f, 7.0f, 7.0f));
	entities.push_back(Entity(playerModel, npc1_matrix));

	generateEntities(&tree1, &terrain, 350, 15.0f);
	generateEntities(&tree2, &terrain, 50, 5.0f);
	generateEntities(&grass, &terrain, 500, 3.0f);
	printf("Done.\n");

	cout << "Enter server url (url:port) ";
	string server;
	getline(cin, server);
	string port = server.substr(server.find(":") + 1, server.size());

	cout << "Enter your name:";
	getline(cin, playerName);

	UDPClient netClient(server.substr(0, server.find(":")), port);
	future<playerInfo> udpPromise = std::async(std::launch::async, &UDPClient::update, netClient, camera.getPosition(), camera.getAngles().y);;

	cout << "ConnectedStatus: " << netClient.connectedStatus << endl;

	Uint32 frameStart;
	int frameTime = 0;
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	while (1)
	{
		bool _break = false;
		frameStart = SDL_GetTicks();

		// User input
		while (SDL_PollEvent(&_event))
			if (camera.checkInputs(_event)) _break = true;

		// Update player
		camera.update(&terrain);

		// Net update
		if (netClient.connectedStatus) {
			if (udpPromise.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
				playerInfo data = udpPromise.get();
				// update player
				if (players.count(data.playerId)) {
					Entity* player = players.at(data.playerId);
					player->modelMatrix = glm::translate(glm::mat4(1), glm::vec3(data.modelMatrix.x, data.modelMatrix.y - camera.playerHeight, data.modelMatrix.z));
					player->modelMatrix = glm::rotate(player->modelMatrix, glm::radians(-data.modelMatrix.w), glm::vec3(0, 1, 0));
					player->modelMatrix = glm::scale(player->modelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));

				}
				// create new player entity from cache in loader
				else {
					cout << "New Player joined server: " << data.playerId << endl;

					glm::mat4 tempModelMatrix = glm::translate(glm::mat4(1), glm::vec3(data.modelMatrix.x, data.modelMatrix.y - camera.playerHeight, data.modelMatrix.z));
					tempModelMatrix = glm::rotate(tempModelMatrix, glm::radians(-data.modelMatrix.w), glm::vec3(0, 1, 0));
					tempModelMatrix = glm::scale(tempModelMatrix, glm::vec3(5.0f, 5.0f, 5.0f));


					Entity* newPlayer = new Entity(playerModel, tempModelMatrix);
					players.insert(pair<string, Entity*>(data.playerId, newPlayer));

					cout << "New Player spawned at: " << glm::to_string(data.modelMatrix) << endl;
				}
				udpPromise = std::async(std::launch::async, &UDPClient::update, netClient, camera.getPosition(), camera.getAngles().y);
			}
		}

		// render
		glClearColor(0.5f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.FOV), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 2500.0f);
		glm::mat4 view = camera.getViewMatrix();

		// RENDER SKYBOX
		glDepthMask(GL_FALSE);
		SkyboxShader.use();
		SkyboxShader.setMat4("projMatrix", projection);
		SkyboxShader.setBool("fadeHorizon", true);

		skybox.Draw(SkyboxShader, camera, SDL_GetTicks() - frameTime);
		glDepthMask(GL_TRUE);

		// RENDER TERRAIN
		TerrainShader.use();

		TerrainShader.setVec3("lightPosition", g_light);
		TerrainShader.setMat4("projMatrix", projection);
		TerrainShader.setMat4("viewMatrix", view);
		TerrainShader.setMat4("modelMatrix", terrainModelMatrix);
		terrain.Draw(TerrainShader);

		// RENDER ENTITIES
		EntityShader.use();

		EntityShader.setVec3("lightPosition", g_light);
		EntityShader.setMat4("projMatrix", projection);
		EntityShader.setMat4("viewMatrix", view);

		for (Entity& e : entities) {
			EntityShader.setMat4("modelMatrix", e.modelMatrix);
			e.Draw(EntityShader);
		}

		for (const auto pair : players) {
			Entity* player = pair.second;
			EntityShader.setMat4("modelMatrix", player->modelMatrix);
			player->Draw(EntityShader);
		}


		// Display
		SDL_GL_SwapWindow(_window);

		// Sleep
		frameTime = SDL_GetTicks() - frameStart;
		if (frameDelay > frameTime)
			SDL_Delay(frameDelay - frameTime);


		if (_break) break;
	}

	// Close window
	SDL_Quit();
	SDL_DestroyWindow(_window);
	SDL_GL_DeleteContext(_context);

	return 0;
}