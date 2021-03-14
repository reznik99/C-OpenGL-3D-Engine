#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL\glew.h>
#include <STB\stb_image.h>
#include "Entity.h"
#include "Terrain.h"

using namespace std;

unsigned int loadTexture(const char* textureFile);

unsigned int loadCubeMapTexture(vector<string> textureFiles);

Entity* readOBJ(const char* filename, const char* textureFile, const char* textureNormalFile, glm::mat4 modelMatrix);

Entity* readOBJ_better(const char* filename, const char* textureFile, const char* textureNormalFile, glm::mat4 modelMatrix);

void cacheEntity(Entity* newEntity, const char* filename);

string readShader(const char* filename);

void genTerrain(const char* heightMapFile, vector<string> textures, glm::mat4 modelMatrix, Terrain* newTerrain, bool flat);

glm::vec3 calculateNormal(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT);

float getHeight(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT);