
#include "Entity.h"

Entity::Entity() {
	modelMatrix = glm::mat4(1.0f);
}
void Entity::load(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals, glm::mat4 *_modelMatrix) {

	unsigned int _indexBufferId = 0;
	// create VAO
	glGenVertexArrays(1, &VAO); //size
	glBindVertexArray(VAO);	// bind the Vertex Array Object first, then bind and set VBO's, and then configure vertex attributes(s).

	//indices
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
	//vertices
	Entity::storeDataInAttributeList(0, 3, _data);
	//normals
	Entity::storeDataInAttributeList(1, 3, _normals);


	glBindVertexArray(0); //unbind
	indexBufferSize = _indices.size();

	if(_modelMatrix == NULL)
		this->modelMatrix = glm::mat4(1.0f); //default model matrix
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

// creates a VBO and binds it to current VAO
void Entity::storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data) {
	unsigned int _bufferId = 0;
	glGenBuffers(1, &_bufferId); //gen vbo
	glBindBuffer(GL_ARRAY_BUFFER, _bufferId); //bind vbo
	glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW); //push data to vbo
	glEnableVertexAttribArray(attributeNumber); //enable it in shader
	glVertexAttribPointer(attributeNumber, coordinateSize, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

