#include "Loader.h"

unsigned int loadTexture(const char* textureFile) {

	unsigned int textureId;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, 0);
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
	unsigned int called = 0;

	std::ifstream file(filename);
	std::string str;
	if (!file) throw _STDEXCEPT_;

	while (std::getline(file, str)) {
		const char* lineHeader = str.c_str();

		if (str.rfind("vt ", 0) == 0) { called++;
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
			if (count != 9) {
				std::cout << "Invalid OBJ format" << std::endl;
				//printf("%s", lineHeader);
			}
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

	std::vector < float > orderedNormals, orderedTextureCoords;

	for (unsigned int i = 0; i < indices.size(); i++) {
		unsigned int currentVertexPointer = indices[i];
		unsigned int currentNormalPointer = normalIndices[i];
		unsigned int currentTexturePointer = textureIndices[i];
		//Normals
		orderedNormals.insert(orderedNormals.begin() + (currentVertexPointer * 3), normals[currentNormalPointer * 3]);
		orderedNormals.insert(orderedNormals.begin() + (currentVertexPointer * 3 + 1), normals[currentNormalPointer * 3 + 1]);
		orderedNormals.insert(orderedNormals.begin() + (currentVertexPointer * 3 + 2), normals[currentNormalPointer * 3 + 2]);
		//UVs
		orderedTextureCoords.insert(orderedTextureCoords.begin() + (currentVertexPointer * 2), textureCoords[currentTexturePointer * 2]);
		orderedTextureCoords.insert(orderedTextureCoords.begin() + (currentVertexPointer * 2 + 1), textureCoords[currentTexturePointer * 2 + 1]);
	}

	std::cout << "Parsed obj successfully." << std::endl;

	//load texture into opengl
	unsigned int textureId = loadTexture(textureFile);
	//save Id's to Entity
	Entity *newEntity = new Entity(vertices, indices, orderedNormals, orderedTextureCoords, &modelMatrix, textureId);

	std::cout << "Loaded Entity successfully" << std::endl;

	return newEntity;
}

std::string readShader(const char* filename) {
	std::ifstream ifs(filename);
	return std::string((std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()));
}