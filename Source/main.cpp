
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
Camera camera(glm::vec3(200, 10, 200), glm::vec3(0, 180, 0));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;



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
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	//stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	printf("Loading Shaders...\n");
	Shader SkyboxShader("./Engine/Shaders/Skybox-Vertex.glsl", "./Engine/Shaders/Skybox-Fragment.glsl");
	Shader TerrainShader("./Engine/Shaders/Terrain-Vertex.glsl", "./Engine/Shaders/Terrain-Fragment.glsl");
	Shader EntityShader("./Engine/Shaders/Entity-Vertex.glsl", "./Engine/Shaders/Entity-Fragment.glsl");

	// Generate terrain
	printf("Generating Terrain...\n");
	string terrainPath = "./Engine/Models/Terrain/";

	vector<string> terrainTextures{ terrainPath + "Blendmap.png",terrainPath + "Grass.png",
		terrainPath + "Grass2.png", terrainPath + "Sand.png", terrainPath + "Rock.png" };
	glm::mat4 terrainModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -4.0f, 0.0f));

	Terrain terrain;
	genTerrain((terrainPath + "heightmap2_scaled.png").c_str(), terrainTextures, terrainModelMatrix, &terrain, false);

	// Set Global illumination
	glm::vec3 g_light = glm::vec3(25.0f, terrain.mapSize / 2, terrain.mapSize * 2); //sunset position (match skybox)

	// Load Skybox
	string skyboxPath = "./Engine/Models/Skybox/Forest/";
	vector<string> files = { skyboxPath + "right.jpg", skyboxPath + "left.jpg", skyboxPath + "top.jpg",
			skyboxPath + "bottom.jpg", skyboxPath + "back.jpg", skyboxPath + "front.jpg" };
	Skybox skybox(files);

	// Load models
	printf("Loading Models...\n");

	Model gs_inn = Model("./Engine/Models/Goldshirehouse/goldshireinn.obj");
	glm::mat4 gs_inn_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(200, terrain.getHeightAt(200, 100) - 5.0f, 100));
	gs_inn_matrix = glm::scale(gs_inn_matrix, glm::vec3(3.5f, 3.5f, 3.5f));
	gs_inn_matrix = glm::rotate(gs_inn_matrix, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	entities.push_back(Entity(&gs_inn, gs_inn_matrix));

	Model evelwyn_tree1 = Model("./Engine/Models/Evelwyn-Tree1/elwynntreecanopy01.obj");
	glm::mat4 evelwyn_tree1_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(200, terrain.getHeightAt(200, 300) - 1.0f, 300));
	evelwyn_tree1_matrix = glm::scale(evelwyn_tree1_matrix, glm::vec3(3.5f, 3.5f, 3.5f));
	entities.push_back(Entity(&evelwyn_tree1, evelwyn_tree1_matrix));

	//Model modelPlayer = Model("./Engine/Models/Orc/orcmalescale.obj");
	playerModel = new Model("./Engine/Models/Orc/orcmalescale.obj");
	glm::mat4 player1_matrix = glm::translate(glm::mat4(1.0f), glm::vec3(250, terrain.getHeightAt(250, 250) - 1.0f, 250));
	player1_matrix = glm::scale(player1_matrix, glm::vec3(7.0f, 7.0f, 7.0f));
	entities.push_back(Entity(playerModel, player1_matrix));

	
	cout << "Enter server url (url:port) ";
	string server;
	getline(cin, server);
	string port = server.substr(server.find(":") + 1, server.size());

	cout << "Enter your name:";
	getline(cin, playerName);

	UDPClient* netClient = new UDPClient(server.substr(0, server.find(":")), port);
	future<playerInfo> udpPromise = std::async(std::launch::async, &UDPClient::update, netClient, camera.getPosition(), camera.getAngles().y);;

	cout << "ConnectedStatus: " << netClient->connectedStatus << endl;

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
		if (netClient->connectedStatus) {
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
		SkyboxShader.setBool("fadeHorizon", false);

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