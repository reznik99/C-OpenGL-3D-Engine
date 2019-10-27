#pragma once

#include <GL\glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

class Terrain
{
public:

	Terrain();

	void load(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals,
		std::vector<float>& _texCoords, glm::mat4& _modelMatrix, unsigned int textureId, std::vector<std::vector<float>>& _heights);

	static unsigned int storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data);
	float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos);

	float getHeightAt(int x, int z);

	const float mapSize = 200;
	const float MAX_HEIGHT = 50;

	unsigned int VAO = 0;

	unsigned int vertVBOId = 0;
	unsigned int normVBOId = 0;
	unsigned int texVBOId = 0;

	unsigned int textureId = 0;
	unsigned int indexBufferSize = 0;

	glm::mat4 modelMatrix; //position, rotation and scale of entity

private:
	std::vector<std::vector<float>> heights;
};

