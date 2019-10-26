#include "Loader.h"

unsigned int loadTexture(const char* textureFile) {

	unsigned int textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, STBI_rgb_alpha);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	std::cout << "Loaded texture" << std::endl;

	return textureId;
}

Entity* readOBJ(const char* filename, const char* textureFile, glm::mat4 modelMatrix) {
	std::cout << "Reading...:  " << filename << std::endl;

	//indices (0 indexed)
	std::vector < unsigned int > indices, normalIndices, textureIndices;
	//vertex data
	std::vector < float > vertices;
	std::vector < float > normals;
	std::vector < float > textureCoords;

	unsigned int vertexCount = 0;

	std::ifstream file(filename);
	std::string str;
	if (!file) throw _STDEXCEPT_;

	while (std::getline(file, str)) {
		const char* lineHeader = str.c_str();

		if (str.rfind("vt ", 0) == 0) {
			float texX, texY;
			sscanf_s(lineHeader, "vt %f %f\n", &texX, &texY);
			textureCoords.push_back(texX);
			textureCoords.push_back(texY);
		}
		else if (str.rfind("vn ", 0) == 0) {
			float nX, nY, nZ;
			sscanf_s(lineHeader, "vn %f %f %f\n", &nX, &nY, &nZ);
			normals.push_back(nX);
			normals.push_back(nY);
			normals.push_back(nZ);
		}
		if (str.rfind("v ", 0) == 0) { vertexCount++;
			float x, y, z;
			sscanf_s(lineHeader, "v %f %f %f\n", &x, &y, &z);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
		else if (str.rfind("f ", 0) == 0) {
			unsigned int v1, t1, n1, v2, t2, n2, v3, t3, n3;
			int count = sscanf_s(lineHeader, "f %d/%d/%d %d/%d/%d %d/%d/%d\n", &v1, &t1, &n1, &
				v2, &t2, &n2, &v3, &t3, &n3);
			if (count != 9)
				std::cout << "Invalid OBJ format" << std::endl;

			indices.push_back(v1 - 1);
			indices.push_back(v2 - 1);
			indices.push_back(v3 - 1);

			textureIndices.push_back(t1 - 1);
			textureIndices.push_back(t2 - 1);
			textureIndices.push_back(t3 - 1);

			normalIndices.push_back(n1 - 1);
			normalIndices.push_back(n2 - 1);
			normalIndices.push_back(n3 - 1);
		}
	}

	std::cout << "Parsed obj successfully." << std::endl;

	//Code to sort attributes according to indices (not fully working, to be used with glDrawArrays())
	/*
	int indicesSize = indices.size();
	std::vector < float > normalsOut;
	std::vector < float > textureCoordsOut;
	normalsOut.resize(indices.size());
	textureCoordsOut.resize(indices.size());

	for (int i = 0; i < indicesSize; i++) {
		int vertPointer = indices.at(i);
		int normPointer = normalIndices.at(i);
		int texPointer = textureIndices.at(i);
		//if already set, duplicate vertex
		if (normalsOut[i] || textureCoordsOut[i]) {
			//update current vertPointer
			int oldVertPointer = vertPointer;
			vertPointer = vertices.size() / 3;
			//add new vertex
			vertices.push_back(vertices[oldVertPointer * 3]);
			vertices.push_back(vertices[oldVertPointer * 3 + 1]);
			vertices.push_back(vertices[oldVertPointer * 3 + 2]);
		}
		//Normal
		normalsOut.insert(normalsOut.begin() + vertPointer * 3, normals[normPointer * 3]);
		normalsOut.insert(normalsOut.begin() + vertPointer * 3 + 1, normals[normPointer * 3 + 1]);
		normalsOut.insert(normalsOut.begin() + vertPointer * 3 + 2, normals[normPointer * 3 + 2]);
		//UV
		textureCoordsOut.insert(textureCoordsOut.begin() + vertPointer * 2, textureCoords[texPointer * 2]);
		textureCoordsOut.insert(textureCoordsOut.begin() + vertPointer * 2 + 1, textureCoords[texPointer * 2 + 1]);
	}*/

	std::cout << "UVs and Normals sorted successfully" << std::endl;

	//load texture into opengl
	unsigned int textureId = loadTexture(textureFile);
	//save Id's to Entity
	Entity *newEntity = new Entity(vertices, indices, normals, textureCoords, &modelMatrix, textureId);

	std::cout << "Loaded Entity successfully" << std::endl;

	return newEntity;
}

std::string readShader(const char* filename) {
	std::ifstream ifs(filename);
	return std::string((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
}

Entity* genTerrain(unsigned int mapSize, float MAX_HEIGHT, const char* heightMapFile, glm::mat4 modelMatrix) {

	//read heightMapFile
	int width, height, nrChannels;
	unsigned char* data = stbi_load(heightMapFile, &width, &height, &nrChannels, STBI_rgb_alpha);
	if (!data)
		std::cout << "Failed to load Heightmap!" << std::endl;

	int VERTEX_COUNT = height/5; //assuming square image
	float MAX_PIXEL_COLOUR = 256 * 256 * 256;

	//generate vertices, normals uvs
	std::vector<float> vertices(VERTEX_COUNT * VERTEX_COUNT * 3);
	std::vector<float> normals(VERTEX_COUNT * VERTEX_COUNT * 3);
	std::vector<float> uvs(VERTEX_COUNT * VERTEX_COUNT * 2);

	int vertexPointer = 0;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {
			//vertices
			vertices[vertexPointer * 3] = (float)j / ((float)VERTEX_COUNT - 1) * mapSize;
			vertices[vertexPointer * 3 + 1] = getHeight(i, j, data, height, nrChannels, MAX_HEIGHT);
			vertices[vertexPointer * 3 + 2] = (float)i / ((float)VERTEX_COUNT - 1) * mapSize;
			//normals
			glm::vec3 normal = glm::vec3(0, 1, 0);//calculateNormal(j, i, image);
			normal = calculateNormal(i, j, data, height, nrChannels, MAX_HEIGHT);
			normals[vertexPointer * 3] = normal.x;
			normals[vertexPointer * 3 + 1] = normal.y;
			normals[vertexPointer * 3 + 2] = normal.z;
			//uvs
			uvs[vertexPointer * 2] = (float)j / ((float)VERTEX_COUNT - 1);
			uvs[vertexPointer * 2 + 1] = (float)i / ((float)VERTEX_COUNT - 1);
			vertexPointer++;
		}
	}
	//generate indices
	std::vector<unsigned int> indices(VERTEX_COUNT * VERTEX_COUNT * 6);
	int pointer = 0;
	for (int gz = 0; gz < VERTEX_COUNT - 1; gz++) {
		for (int gx = 0; gx < VERTEX_COUNT - 1; gx++) {
			int topLeft = (gz * VERTEX_COUNT) + gx;
			int topRight = topLeft + 1;
			int bottomLeft = ((gz + 1) * VERTEX_COUNT) + gx;
			int bottomRight = bottomLeft + 1;
			indices[pointer++] = topLeft;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = topRight;
			indices[pointer++] = topRight;
			indices[pointer++] = bottomLeft;
			indices[pointer++] = bottomRight;
		}
	}

	//Read textures (normal, textures, blendmap)
	int texId = loadTexture("../Debug/rock.png");

	//generate Entity with data
	Entity *newEntity = new Entity(vertices, indices, normals, uvs, &modelMatrix, texId);

	stbi_image_free(data);
	
	return newEntity;
}

glm::vec3 calculateNormal(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT) {
	float heightL = getHeight(i - 1, j, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightR = getHeight(i + 1, j, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightD = getHeight(i, j - 1, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightU = getHeight(i, j + 1, heightMap, height, nrChannels, MAX_HEIGHT);
	glm::vec3 normal = glm::vec3(heightL - heightR, 2.0f, heightD - heightU);
	return glm::normalize(normal);
}

float getHeight(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT) {
	if (i < 0 || i >= height || j < 0 || j >= height) return NULL;

	unsigned char* pixelOffset = heightMap + (i + height * j) * nrChannels;
	unsigned char r = pixelOffset[0];
	unsigned char g = pixelOffset[1];
	unsigned char b = pixelOffset[2];
	float currentHeight = ((r + g + b) / 3.0f) / 255.0f;

	return currentHeight * MAX_HEIGHT - MAX_HEIGHT;
}