#pragma once
#include "shader.h"
#include "model.h"

class Entity
{
public:
	

	Entity(Model* model, glm::mat4 matrix) {
		entityModel = model;
		modelMatrix = matrix;
	}

	void Draw(Shader& shader) {
		entityModel->Draw(shader);
	}

	Model* entityModel;			// Pointer to model (shared between many Entities EG. Trees)
	glm::mat4 modelMatrix;		// ModelMatrix, Unique among all entites


};