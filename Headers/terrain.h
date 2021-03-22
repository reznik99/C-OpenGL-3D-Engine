#pragma once

#include <GL\glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

#include "shader.h"

class Terrain
{
public:

	Terrain() {}

	void Draw(Shader& shader) {
		//load uniform for model matrix
		int modelMatrixId = glGetUniformLocation(shader.ID, "modelMatrix");
		glUniformMatrix4fv(modelMatrixId, 1, GL_FALSE, &this->modelMatrix[0][0]);
		glUniform1i(glGetUniformLocation(shader.ID, "blendMapTex"), 0);
		glUniform1i(glGetUniformLocation(shader.ID, "tex1"), 1);
		glUniform1i(glGetUniformLocation(shader.ID, "tex2"), 2);
		glUniform1i(glGetUniformLocation(shader.ID, "tex3"), 3);
		glUniform1i(glGetUniformLocation(shader.ID, "tex4"), 4);
		//bind texture
		for (unsigned int i = 0; i < this->textureIds.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, this->textureIds.at(i));
		}
		//bind vao
		glBindVertexArray(this->VAO);
		//render
		glDrawElements(GL_TRIANGLES, this->indexBufferSize, GL_UNSIGNED_INT, 0);
		//unbind
		glBindVertexArray(0);
	}

	void load(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals,
		std::vector<float>& _texCoords, glm::mat4& _modelMatrix, std::vector<int>& textureIds, std::vector<std::vector<float>>& _heights) {

		unsigned int _indexBufferId = 0;
		// create VAO
		glGenVertexArrays(1, &VAO); //generate vao
		glBindVertexArray(VAO);	// bind the Vertex Array Object first, then bind and set VBO's, and then configure vertex attributes(s).

		//indices
		glGenBuffers(1, &_indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
		//vertices
		this->VBOs.push_back(Terrain::storeDataInAttributeList(0, 3, _data));
		//normals
		this->VBOs.push_back(Terrain::storeDataInAttributeList(1, 3, _normals));
		//textureCoords
		this->VBOs.push_back(Terrain::storeDataInAttributeList(2, 2, _texCoords));

		this->textureIds = textureIds;
		this->indexBufferSize = (unsigned int)_indices.size();
		this->modelMatrix = _modelMatrix;

		this->heights = _heights;

		glBindVertexArray(0); //unbind
	}

	// creates a VBO and binds it to current VAO
	unsigned int storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data) {
		unsigned int _bufferId = 0;
		glGenBuffers(1, &_bufferId); //gen vbo
		glBindBuffer(GL_ARRAY_BUFFER, _bufferId); //bind vbo
		glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW); //push data to vbo
		glEnableVertexAttribArray(attributeNumber); //enable it in shader
		glVertexAttribPointer(attributeNumber, coordinateSize, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		return _bufferId;
	}

	float barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {
		float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
		float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
		float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
		float l3 = 1.0f - l1 - l2;
		return l1 * p1.y + l2 * p2.y + l3 * p3.y;
	}

	float getHeightAt(int x, int z) {

		float gridSquareSize = mapSize / (heights.size() - 1);
		unsigned int gridX = (int)floor(x / gridSquareSize);
		unsigned int gridZ = (int)floor(z / gridSquareSize);
		if (gridX < 0 || gridZ < 0 || gridX >= heights.size() - 1 || gridZ >= heights.size() - 1)
			return NULL;

		float xCoord = (float)fmod(x, gridSquareSize) / gridSquareSize;//(x % (int)gridSquareSize) / gridSquareSize;
		float zCoord = (float)fmod(z, gridSquareSize) / gridSquareSize;
		float answer;
		if (xCoord <= (1 - zCoord)) {
			answer = Terrain::barryCentric(glm::vec3(0, heights[gridX][gridZ], 0), glm::vec3(1,
				heights[gridX + 1][gridZ], 0), glm::vec3(0,
					heights[gridX][gridZ + 1], 1), glm::vec2(xCoord, zCoord));
		}
		else {
			answer = Terrain::barryCentric(glm::vec3(1, heights[gridX + 1][gridZ], 0), glm::vec3(1,
				heights[gridX + 1][gridZ + 1], 1), glm::vec3(0,
					heights[gridX][gridZ + 1], 1), glm::vec2(xCoord, zCoord));
		}

		return answer;
	}

	const float mapSize = 2600;
	const float MAX_HEIGHT = 200;

	unsigned int VAO = 0;
	std::vector<unsigned int> VBOs = std::vector<unsigned int>{};

	std::vector<int> textureIds = std::vector<int>{};
	unsigned int indexBufferSize = 0;

	glm::mat4 modelMatrix; //position, rotation and scale of entity

private:
	std::vector<std::vector<float>> heights = std::vector<std::vector<float>>{};
};

