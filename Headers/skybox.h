#pragma once

#include <GL\glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

#include "shader.h"
#include "camera.h"
#include "loader.h"
using namespace std;

class Skybox
{
public:

	Skybox() {

		// create buffers/arrays
		glGenBuffers(1, &this->skyboxVBO);

		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, this->skyboxVBO);

		glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(float), &this->vertices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindVertexArray(0);

		// Load Cube map textures
		vector<string> files = { path + "right.jpg", path + "left.jpg", path + "top.jpg",
			path + "bottom.jpg", path + "back.jpg", path + "front.jpg" };
		this->cubeMapTextureId = loadCubeMapTexture(files);
	}

	void Draw(Shader& shader, Camera camera, int frameStart) {
		this->rotation = windSpeed * (frameStart / 1000.0f);

		glm::mat4 skyboxViewMatrix = camera.getViewMatrix();
		skyboxViewMatrix[3][0] = 0;
		skyboxViewMatrix[3][1] = 0;		//so player never moves closer to skybox
		skyboxViewMatrix[3][2] = 0;
		skyboxViewMatrix = glm::rotate(skyboxViewMatrix, glm::radians(this->rotation), glm::vec3(0, 1, 0));

		shader.setMat4("viewMatrix", skyboxViewMatrix);

		//bind texture
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, this->cubeMapTextureId);

		glBindBuffer(GL_ARRAY_BUFFER, this->skyboxVBO);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(this->vertices.size()));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glUseProgram(0);
	}

private:
	const float windSpeed = 1.0f;
	float rotation = 1.0f;

	// Skybox Model
	unsigned int cubeMapTextureId;
	unsigned int skyboxVBO;
	const string path = "Models/Skybox/Forest/";

	// Skybox model
	const float size = 1000.0f;
	std::vector<float> vertices{
		-size,  size, -size,
		-size, -size, -size,
		size, -size, -size,
		 size, -size, -size,
		 size,  size, -size,
		-size,  size, -size,

		-size, -size,  size,
		-size, -size, -size,
		-size,  size, -size,
		-size,  size, -size,
		-size,  size,  size,
		-size, -size,  size,

		 size, -size, -size,
		 size, -size,  size,
		 size,  size,  size,
		 size,  size,  size,
		 size,  size, -size,
		 size, -size, -size,

		-size, -size,  size,
		-size,  size,  size,
		 size,  size,  size,
		 size,  size,  size,
		 size, -size,  size,
		-size, -size,  size,

		-size,  size, -size,
		 size,  size, -size,
		 size,  size,  size,
		 size,  size,  size,
		-size,  size,  size,
		-size,  size, -size,

		-size, -size, -size,
		-size, -size,  size,
		 size, -size, -size,
		 size, -size, -size,
		-size, -size,  size,
		 size, -size,  size
	};

};

