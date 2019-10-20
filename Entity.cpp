
#include "Entity.h"

Entity::Entity() {
	modelMatrix = glm::mat4(1.0f);
}
void Entity::load(std::vector<float>& _data, std::vector<unsigned int>& _indices) {

	unsigned int _bufferId = 0, _indexBufferId = 0;

	// create VBOs
	//normals, texture_cords, color, etc.
	//vertices
	glGenBuffers(1, &_bufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _bufferId);
	glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
	//indices
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW

	// create VAO
	glGenVertexArrays(2, &VAO); //size
	glBindVertexArray(VAO);	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).

	//add vertex VBO to VAO
	glBindBuffer(GL_ARRAY_BUFFER, _bufferId);	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//add indices VBO to VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_UNSIGNED_INT, GL_FALSE, 0, 0);

	glBindVertexArray(0); //unbind
	indexBufferSize = _indices.size();

	modelMatrix = glm::mat4(1.0f);

	std::cout << "VAO: " << VAO << std::endl;
}

int Entity::update() {
	modelMatrix = glm::rotate(modelMatrix, 3.14f / 200, glm::vec3(0, 1.0, 0));

	return 0;
}

