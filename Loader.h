#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL\glew.h>
#include <STB\stb_image.h>
#include <Entity.h>
#include <Terrain.h>
#include <map>


unsigned int loadTexture(const char* textureFile);

Entity* readOBJ(const char* filename, const char* textureFile, glm::mat4 modelMatrix);

std::string readShader(const char* filename);

void genTerrain(const char* heightMapFile, const char* textureFile1, glm::mat4 modelMatrix, Terrain* newTerrain);

glm::vec3 calculateNormal(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT);

float getHeight(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT);