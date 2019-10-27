
#include "Entity.h"

Entity::Entity() {
	this->modelMatrix = this->modelMatrix = glm::mat4(1.0f); //default model matrix
}

Entity::Entity(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals,
	std::vector<float>& _texCoords, glm::mat4 *_modelMatrix, unsigned int textureId, unsigned int normalTextureId) {

	unsigned int _indexBufferId = 0;
	// create VAO
	glGenVertexArrays(1, &VAO); //generate vao
	glBindVertexArray(VAO);	// bind the Vertex Array Object first, then bind and set VBO's, and then configure vertex attributes(s).

	//indices
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
	//vertices
	this->vertVBOId = Entity::storeDataInAttributeList(0, 3, _data);
	//normals
	this->normVBOId = Entity::storeDataInAttributeList(1, 3, _normals);
	//textureCoords
	this->texVBOId = Entity::storeDataInAttributeList(2, 2, _texCoords);

	this->textureId = textureId;
	this->normalTextureId = normalTextureId;
	this->indexBufferSize = _indices.size();

	glBindVertexArray(0); //unbind
	
	this->modelMatrix = *_modelMatrix;

	//debuggin purposes
	std::cout << "VAO: " << VAO << std::endl;
	std::cout << "Indices: " << _indices.size() << std::endl;
	std::cout << "Vertices: " << _data.size() << std::endl;
	std::cout << "Normals: " << _normals.size() << std::endl;
	std::cout << "textureCoords: " << _texCoords.size() << std::endl;
}

void Entity::loadCached(unsigned int _VAO, unsigned int _vertVBOId, unsigned int _normVBOId,
	unsigned int _texVBOId, unsigned int _indexBufferSize, unsigned int _textureId, unsigned int _normalTextureId, glm::mat4* _modelMatrix ) {

	this->VAO = _VAO;
	this->vertVBOId = _vertVBOId;
	this->normVBOId = _normVBOId;
	this->texVBOId = _texVBOId;
	this->textureId = _textureId;
	this->normalTextureId = _normalTextureId;
	this->indexBufferSize = _indexBufferSize;

	this->modelMatrix = *_modelMatrix;

}

int Entity::update() {
	modelMatrix = glm::rotate(modelMatrix, 3.14f / 200, glm::vec3(0, 1.0, 0));

	return 0;
}

// creates a VBO and binds it to current VAO
unsigned int Entity::storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data) {
	unsigned int _bufferId = 0;
	glGenBuffers(1, &_bufferId); //gen vbo
	glBindBuffer(GL_ARRAY_BUFFER, _bufferId); //bind vbo
	glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW); //push data to vbo
	glEnableVertexAttribArray(attributeNumber); //enable it in shader
	glVertexAttribPointer(attributeNumber, coordinateSize, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return _bufferId;
}

