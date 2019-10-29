#pragma once
#include <vector>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GL\glew.h>

class Entity
{
public:

	Entity();

	Entity(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals,
		std::vector<float>& _texCoords, glm::mat4* _modelMatrix, unsigned int textureId, unsigned int textureNormalId);

	void loadCached(unsigned int _VAO, unsigned int _vertVBOId, unsigned int _normVBOId, unsigned int _texVBOId, 
		unsigned int _indexBufferSize, unsigned int _textureId, unsigned int _normalTextureId, glm::mat4* _modelMatrix);

	static unsigned int storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data);

	int update();

	unsigned int VAO = 0;
	std::vector<unsigned int> VBOs;

	unsigned int textureId = 0;
	unsigned int normalTextureId = 0;
	unsigned int indexBufferSize = 0; //for glDrawElements

	glm::mat4 modelMatrix; //position, rotation and scale of entity
};

