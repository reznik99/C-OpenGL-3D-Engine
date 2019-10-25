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

	static unsigned int storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data);

	int update();

	unsigned int VAO = 0;

	unsigned int vertVBOId = 0;
	unsigned int normVBOId = 0;
	unsigned int texVBOId = 0;

	unsigned int textureId = 0;
	unsigned int indexBufferSize = 0; //for glDrawElements

	glm::mat4 modelMatrix; //position, rotation and scale of entity
};

