#include "Terrain.h"

Terrain::Terrain() {

}

void Terrain::load(std::vector<float>& _data, std::vector<unsigned int>& _indices, std::vector<float>& _normals,
	std::vector<float>& _texCoords, glm::mat4& _modelMatrix, unsigned int textureId, std::vector<std::vector<float>>& _heights) {

	unsigned int _indexBufferId = 0;
	// create VAO
	glGenVertexArrays(1, &VAO); //generate vao
	glBindVertexArray(VAO);	// bind the Vertex Array Object first, then bind and set VBO's, and then configure vertex attributes(s).

	//indices
	glGenBuffers(1, &_indexBufferId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indexBufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), &_indices[0], GL_STATIC_DRAW); //STREAM_DRAW and DYNAMIC_DRAW
	//vertices
	this->vertVBOId = Terrain::storeDataInAttributeList(0, 3, _data);
	//normals
	this->normVBOId = Terrain::storeDataInAttributeList(1, 3, _normals);
	//textureCoords
	this->texVBOId = Terrain::storeDataInAttributeList(2, 2, _texCoords);

	this->textureId = textureId;
	this->indexBufferSize = _indices.size();
	this->modelMatrix = _modelMatrix;

	this->heights = _heights;

	glBindVertexArray(0); //unbind
}

// creates a VBO and binds it to current VAO
unsigned int Terrain::storeDataInAttributeList(int attributeNumber, int coordinateSize, std::vector<float>& _data) {
	unsigned int _bufferId = 0;
	glGenBuffers(1, &_bufferId); //gen vbo
	glBindBuffer(GL_ARRAY_BUFFER, _bufferId); //bind vbo
	glBufferData(GL_ARRAY_BUFFER, _data.size() * sizeof(float), &_data[0], GL_STATIC_DRAW); //push data to vbo
	glEnableVertexAttribArray(attributeNumber); //enable it in shader
	glVertexAttribPointer(attributeNumber, coordinateSize, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return _bufferId;
}

float Terrain::getHeightAt(int x, int z) {

	float gridSquareSize = mapSize / (heights.size() - 1);
	int gridX = (int)floor(x / gridSquareSize);
	int gridZ = (int)floor(z / gridSquareSize);
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

float Terrain::barryCentric(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec2 pos) {
	float det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
	float l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
	float l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
	float l3 = 1.0f - l1 - l2;
	return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}

