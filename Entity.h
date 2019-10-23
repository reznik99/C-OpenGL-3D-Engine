#pragma once
#include <vector>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GL\glew.h>

class Entity
{
public:
	Entity(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals,
		std::vector<float>& _texCoords, glm::mat4* _modelMatrix, unsigned int textureId);

	static void storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data);

	int update();

	unsigned int VAO = 0;
	unsigned int textureId = 0;
	unsigned int indexBufferSize = 0;
	glm::mat4 modelMatrix;
};

