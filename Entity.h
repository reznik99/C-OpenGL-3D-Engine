#pragma once
#include <vector>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <GL\glew.h>

class Entity
{
public:
	Entity();
	void load(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals, 
		glm::mat4* _modelMatrix);

	int update();

	unsigned int VAO = 0;
	unsigned int indexBufferSize = 0;
	glm::mat4 modelMatrix;
private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
};

