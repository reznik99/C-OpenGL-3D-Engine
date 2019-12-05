#pragma once
#include <vector>
#include "Terrain.h"
#include "Entity.h"
#include "Camera.h"
#include "Loader.h"

using namespace std;

class Renderer
{
public:
	Renderer(unsigned int width, unsigned int height);

	void render(glm::vec3& light, Camera& camera);

	void update();

	void processEntity(Entity e);

	void loadUniforms(unsigned int _pid, glm::vec3& light, Camera& camera);

	void cleanUp();

	Terrain* getTerrain();

	unsigned int createShader(unsigned int type, const string& source);
	unsigned int createShaderProgram(const string& vertexShaderSource,
		const string& fragmentShaderSource, unsigned int index);

	map<string, Entity*> players;

private:
	const float FOV = 75.0f;

	unsigned int g_EntityProgramId = 0, 
		g_TerrainProgramId = 0,
		g_SkyboxProgramId = 0;

	unsigned int shaderIds[6]; //vertexEntities - fragEntities | vertexTerrain - fragTerrain ... etc

	vector<Entity> entities;
	Terrain terrain;
	glm::mat4 projectionMatrix;

	//skybox stuff (should have own class)
	unsigned int cubeMapTextureId;
	unsigned int skyboxVBO;

	float SIZE = 512.0f;
	vector<float> VERTICES{
		-SIZE,  SIZE, -SIZE,
		-SIZE, -SIZE, -SIZE,
		SIZE, -SIZE, -SIZE,
		 SIZE, -SIZE, -SIZE,
		 SIZE,  SIZE, -SIZE,
		-SIZE,  SIZE, -SIZE,

		-SIZE, -SIZE,  SIZE,
		-SIZE, -SIZE, -SIZE,
		-SIZE,  SIZE, -SIZE,
		-SIZE,  SIZE, -SIZE,
		-SIZE,  SIZE,  SIZE,
		-SIZE, -SIZE,  SIZE,

		 SIZE, -SIZE, -SIZE,
		 SIZE, -SIZE,  SIZE,
		 SIZE,  SIZE,  SIZE,
		 SIZE,  SIZE,  SIZE,
		 SIZE,  SIZE, -SIZE,
		 SIZE, -SIZE, -SIZE,

		-SIZE, -SIZE,  SIZE,
		-SIZE,  SIZE,  SIZE,
		 SIZE,  SIZE,  SIZE,
		 SIZE,  SIZE,  SIZE,
		 SIZE, -SIZE,  SIZE,
		-SIZE, -SIZE,  SIZE,

		-SIZE,  SIZE, -SIZE,
		 SIZE,  SIZE, -SIZE,
		 SIZE,  SIZE,  SIZE,
		 SIZE,  SIZE,  SIZE,
		-SIZE,  SIZE,  SIZE,
		-SIZE,  SIZE, -SIZE,

		-SIZE, -SIZE, -SIZE,
		-SIZE, -SIZE,  SIZE,
		 SIZE, -SIZE, -SIZE,
		 SIZE, -SIZE, -SIZE,
		-SIZE, -SIZE,  SIZE,
		 SIZE, -SIZE,  SIZE
	};

};

