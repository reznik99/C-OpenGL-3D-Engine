
#include "Entity.h"

Entity::Entity() {
	modelMatrix = glm::mat4(1.0f);
}
void Entity::load(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals, glm::mat4 *_modelMatrix) {

	unsigned int _indexBufferId = 0, _vertexBufferId = 0, _normalsBuffer = 0;

	// create VBOs
	//indices
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
	//vertices
	glGenBuffers(1, &_vertexBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);
	glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW);
	//normals
	glGenBuffers(1, &_normalsBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, _normalsBuffer);
	glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(float), &_normals[0], GL_STATIC_DRAW);

	// create VAO
	glGenVertexArrays(3, &VAO); //size
	glBindVertexArray(VAO);	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).

	//add indices VBO to VAO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_UNSIGNED_INT, GL_FALSE, 0, 0);
	//add vertex VBO to VAO
	glBindBuffer(GL_ARRAY_BUFFER, _vertexBufferId);	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//add normals VBO to VAO
	glBindBuffer(GL_ARRAY_BUFFER, _normalsBuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	

	glBindVertexArray(0); //unbind
	indexBufferSize = _indices.size();

	if(_modelMatrix == NULL)
		modelMatrix = glm::mat4(1.0f);
	else
		this->modelMatrix = *_modelMatrix;

	std::cout << "VAO: " << VAO << std::endl;
	std::cout << "Indices: " << _indices.size() << std::endl;
	std::cout << "Vertices: " << _data.size() << std::endl;
	std::cout << "Normals: " << _normals.size() << std::endl;
}

int Entity::update() {
	modelMatrix = glm::rotate(modelMatrix, 3.14f / 200, glm::vec3(0, 1.0, 0));

	return 0;
}

