#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL\glew.h>

#include "Entity.h"
#include "Terrain.h"

using namespace std;

static unsigned int loadTexture(const char* path);
unsigned int loadCubeMapTexture(vector<string> textureFiles);
void genTerrain(const char* heightMapFile, vector<string> textures, glm::mat4 modelMatrix, Terrain* newTerrain, bool flat);
glm::vec3 calculateNormal(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT);
float getHeight(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT);

// SKYBOX

static unsigned int loadTexture(const char* path)
{
	string filename = string(path);

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = GL_RED;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubeMapTexture(vector<string> textureFiles) {
	unsigned int textureId;
	glGenTextures(1, &textureId);
	glActiveTexture(textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	for (unsigned int i = 0; i < textureFiles.size(); i++) {
		const char* textureFile = textureFiles.at(i).c_str();
		// load and generate the texture
		int width, height, nrChannels;
		unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
			cout << "Failed to load texture" << endl;
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return textureId;
}

// TERRAIN

void genTerrain(const char* heightMapFile, vector<string> textures, glm::mat4 modelMatrix, Terrain* newTerrain, bool flat) {

	//read heightMapFile
	int width, height, nrChannels;
	unsigned char* data = stbi_load(heightMapFile, &width, &height, &nrChannels, STBI_grey);
	if (!data)
		cout << "Failed to load Heightmap!" << endl;

	__int64 VERTEX_COUNT = static_cast<__int64>(height); //assuming square image
	float MAX_PIXEL_COLOUR = 256 * 256 * 256;

	//generate vertices, normals uvs
	vector<float> vertices(VERTEX_COUNT * VERTEX_COUNT * 3);
	vector<float> normals(VERTEX_COUNT * VERTEX_COUNT * 3);
	vector<float> uvs(VERTEX_COUNT * VERTEX_COUNT * 2);

	vector<vector<float>> heights;
	heights.resize(VERTEX_COUNT, vector<float>(VERTEX_COUNT, 0));

	__int64 vertexPointer = 0;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {
			//vertices
			float vertHeight = getHeight(i, j, data, height, nrChannels, newTerrain->MAX_HEIGHT);
			heights[i][j] = flat ? 0 : vertHeight; //add height to collision buffer
			vertices[vertexPointer * 3] = (float)j / ((float)VERTEX_COUNT - 1) * newTerrain->mapSize;
			vertices[vertexPointer * 3 + 1] = flat ? 0 : vertHeight;
			vertices[vertexPointer * 3 + 2] = (float)i / ((float)VERTEX_COUNT - 1) * newTerrain->mapSize;
			//normals
			glm::vec3 normal = glm::vec3(0, 1, 0);//calculateNormal(j, i, image);
			normal = flat ? glm::vec3(0, 1, 0) : calculateNormal(i, j, data, height, nrChannels, newTerrain->MAX_HEIGHT);
			normals[vertexPointer * 3] = normal.x;
			normals[vertexPointer * 3 + 1] = normal.y;
			normals[vertexPointer * 3 + 2] = normal.z;
			//uvs
			uvs[vertexPointer * 2] = (float)j / ((float)VERTEX_COUNT - 1);
			uvs[vertexPointer * 2 + 1] = (float)i / ((float)VERTEX_COUNT - 1);
			vertexPointer++;
		}
	}
	//generate indices
	vector<unsigned int> indices(VERTEX_COUNT * VERTEX_COUNT * 6);
	int pointer = 0;
	for (int gz = 0; gz < VERTEX_COUNT - 1; gz++) {
		for (int gx = 0; gx < VERTEX_COUNT - 1; gx++) {
			int topLeft = (gz * static_cast<int>(VERTEX_COUNT)) + gx;
			int topRight = topLeft + 1;
			int bottomLeft = ((gz + 1) * static_cast<int>(VERTEX_COUNT)) + gx;
			int bottomRight = bottomLeft + 1;
			indices[pointer++] = topLeft;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = topRight;
			indices[pointer++] = topRight;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = bottomRight;
		}
	}

	//Read textures (normal, textures, blendmap)
	vector<int> textureIds(5);
	for (unsigned int i = 0; i < textures.size(); i++)
		textureIds[i] = loadTexture(textures.at(i).c_str());

	//load Terrain with data
	newTerrain->load(vertices, indices, normals, uvs, modelMatrix, textureIds, heights);

	stbi_image_free(data);
}

glm::vec3 calculateNormal(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT) {
	float heightD = getHeight(i - 1, j, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightU = getHeight(i + 1, j, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightL = getHeight(i, j - 1, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightR = getHeight(i, j + 1, heightMap, height, nrChannels, MAX_HEIGHT);
	glm::vec3 normal = glm::vec3(heightL - heightR, 2.0f, heightD - heightU);
	return glm::normalize(normal);
}

float getHeight(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT) {
	if (i < 0 || i >= height || j < 0 || j >= height) return NULL;

	unsigned char* pixelOffset = &heightMap[i * height + j];// *nrChannels;
	unsigned char r = pixelOffset[0];
	unsigned char g = pixelOffset[1];
	unsigned char b = pixelOffset[2];
	float currentHeight = ((r + g + b) / 3.0f) / 255.0f;

	return currentHeight * MAX_HEIGHT;
}