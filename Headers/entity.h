#pragma once
#include "shader.h"
#include "model.h"

class Entity
{
public:
	Entity() {}

	Entity(Model& model, glm::mat4 matrix) {
		this->entityModel = model;
		this->modelMatrix = matrix;
	}

	void Draw(Shader& shader) {
		this->entityModel.Draw(shader);
	}

	Model entityModel;
	glm::mat4 modelMatrix;


};