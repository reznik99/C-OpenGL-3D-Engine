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
	std::vector <unsigned int> indices;
	std::vector <unsigned int> normalIndices;
	std::vector <unsigned int> textureIndices;
	//vertex data
	std::vector <float> vertices;
	std::vector <float> normals;
	std::vector <float> textureCoords;

	unsigned int vertexCount = 0;
	unsigned int fileLines = 0;

	std::string s;
	std::ifstream fin(filename);
	if (!fin) throw _STDEXCEPT_;

	while (fin >> s) {
		fileLines++;
		const char* line = s.c_str();
		switch (*line) {
		case 'v': {
			if (*(line + 1) == 'n') {
				float nX, nY, nZ;
				fin >> nX >> nY >> nZ;
				normals.push_back(nX);
				normals.push_back(nY);
				normals.push_back(nZ);
			}
			else if (*(line + 1) == 't') {
				float texX, texY;
				fin >> texX >> texY;
				textureCoords.push_back(texX);
				textureCoords.push_back(texY);
			}
			else {
				vertexCount++;
				float x, y, z;
				fin >> x >> y >> z;
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			}
		}
		break;
		case 'f': {
			unsigned int v1, t1, n1,
				v2, t2, n2,
				v3, t3, n3;
			fin >> v1 >> t1 >> n1 >> v2 >> t2 >> n2 >> v3 >> t3 >> n3;
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
		break;
		}
	}

	std::cout << "Vertex count x 2: " << vertexCount*2 << std::endl;

	unsigned int size = indices.size();
	float* orderedNormals = new float[vertexCount * 3];
	float* orderedTextureCoords = new float[vertexCount * 2];
	//sort Normals and TextureCoords to match 'indices' in vertex shader
	for (unsigned int i = 0; i < size; i++) {
		int currentVertexPointer = indices[i];
		int currentNormalPointer = normalIndices[i];
		int currentTexturePointer = textureIndices[i];
		//Normal at vertex position should equal the normal that face pointed to
		orderedNormals[currentVertexPointer * 3] = normals[currentNormalPointer * 3];
		orderedNormals[currentVertexPointer * 3 + 1] = normals[currentNormalPointer * 3 + 1];
		orderedNormals[currentVertexPointer * 3 + 2] = normals[currentNormalPointer * 3 + 2];
		//Texture cord at vertex position should equal coord pointed by face
		orderedTextureCoords[currentVertexPointer * 2] = textureCoords[currentTexturePointer * 2];
		orderedTextureCoords[currentVertexPointer * 2 + 1] = textureCoords[currentTexturePointer * 2 + 1];
	}

	//convert ordered attributes to vectors again
	std::vector <float> finalNormals(orderedNormals, orderedNormals + vertexCount * 3);
	std::vector <float> finalTextureCoords(orderedTextureCoords, orderedTextureCoords + vertexCount * 2);

	//clean up temporary arrays
	delete[] orderedNormals;
	delete[] orderedTextureCoords;

	std::cout << "Parsed obj successfully." << std::endl;

	//load texture into opengl
	unsigned int textureId = loadTexture(textureFile);
	//save Id's to Entity
	Entity* newEntity = new Entity(vertices, indices, finalNormals, finalTextureCoords, &modelMatrix, textureId);

	std::cout << "Loaded Entity successfully" << std::endl;

	return newEntity;
}

Entity* readOBJ_better(const char* filename, const char* textureFile, glm::mat4 modelMatrix) {
	std::cout << "Reading...:  " << filename << std::endl;

	//indices (0 indexed)
	std::vector <unsigned int> indices, normalIndices, textureIndices;
	//vertex data
	std::vector <float> vertices;
	std::vector <float> normals;
	std::vector <float> textureCoords;

	unsigned int vertexCount = 0;
	FILE* file;
	errno_t err;
	err = fopen_s(&file, filename, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, sizeof(lineHeader) * sizeof(char));
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		// else : parse lineHeader
		if (strcmp(lineHeader, "v") == 0) {
			vertexCount++;
			float x, y, z;
			fscanf_s(file, "%f %f %f\n", &x, &y, &z);
			vertices.push_back(x);
			vertices.push_back(y);
			vertices.push_back(z);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			float texX, texY;
			fscanf_s(file, "%f %f\n", &texX, &texY);
			textureCoords.push_back(texX);
			textureCoords.push_back(texY);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			float nX, nY, nZ;
			fscanf_s(file, "%f %f %f\n", &nX, &nY, &nZ);
			normals.push_back(nX);
			normals.push_back(nY);
			normals.push_back(nZ);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			unsigned int v1, t1, n1, v2, t2, n2, v3, t3, n3;
			int count = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &v1, &t1, &n1,
				&v2, &t2, &n2, &v3, &t3, &n3);
			if (count != 9) {
				std::cout << "Invalid OBJ format" << std::endl;
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

	std::cout << "Vertex count x 2: " << vertexCount * 2 << std::endl;

	unsigned int size = indices.size();
	float* orderedNormals = new float[vertexCount * 3];
	float* orderedTextureCoords = new float[vertexCount * 2];
	//sort Normals and TextureCoords to match 'indices' in vertex shader
	for (unsigned int i = 0; i < size; i++) {
		int currentVertexPointer = indices[i];
		int currentNormalPointer = normalIndices[i];
		int currentTexturePointer = textureIndices[i];
		//Normal at vertex position should equal the normal that face pointed to
		orderedNormals[currentVertexPointer * 3] = normals[currentNormalPointer * 3];
		orderedNormals[currentVertexPointer * 3 + 1] = normals[currentNormalPointer * 3 + 1];
		orderedNormals[currentVertexPointer * 3 + 2] = normals[currentNormalPointer * 3 + 2];
		//Texture cord at vertex position should equal coord pointed by face
		orderedTextureCoords[currentVertexPointer * 2] = textureCoords[currentTexturePointer * 2];
		orderedTextureCoords[currentVertexPointer * 2 + 1] = textureCoords[currentTexturePointer * 2 + 1];
	}

	//convert ordered attributes to vectors again
	std::vector <float> finalNormals(orderedNormals, orderedNormals + vertexCount * 3);
	std::vector <float> finalTextureCoords(orderedTextureCoords, orderedTextureCoords + vertexCount * 2);

	//clean up temporary arrays
	delete[] orderedNormals;
	delete[] orderedTextureCoords;

	std::cout << "Parsed obj successfully." << std::endl;

	//load texture into opengl
	unsigned int textureId = loadTexture(textureFile);
	//save Id's to Entity
	Entity* newEntity = new Entity(vertices, indices, finalNormals, finalTextureCoords, &modelMatrix, textureId);

	std::cout << "Loaded Entity successfully" << std::endl;

	return newEntity;
}