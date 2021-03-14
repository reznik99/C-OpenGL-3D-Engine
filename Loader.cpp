#include "Loader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

map<string, vector<unsigned int>> cache;

struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
	}
};

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
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1);
	}
	else {
		cout << "Failed to load texture " << textureFile << endl;
	}
	stbi_image_free(data);

	return textureId;
}

unsigned int loadCubeMapTexture(vector<string> textureFiles) {
	unsigned int textureId;
	glGenTextures(1, &textureId);
	glActiveTexture(textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

	for (unsigned int i = 0; i < textureFiles.size(); i++) {
		const char* textureFile = textureFiles.at(i).c_str();
		// load and generate the texture
		int width, height, nrChannels;
		unsigned char* data = stbi_load(textureFile, &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		else
			cout << "Failed to load texture" << endl;
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return textureId;
}

Entity* readOBJ(const char* filename, const char* textureFile, const char* textureNormalFile, glm::mat4 modelMatrix) {
	//if loaded obj before, don't load again!
	if (cache.count(filename)) {
		cout << "Cache Hit!...:  " << filename << endl;
		vector<unsigned int> ids(cache.at(filename));
		Entity* cachedEntity = new Entity();
		cachedEntity->loadCached(ids[0], ids[1], ids[2], ids[3], ids[4], ids[5], ids[6], &modelMatrix);

		return cachedEntity;
	}

	cout << "Reading...:  " << filename << endl;

	//indices (0 indexed)
	vector < unsigned int > indices, normalIndices, textureIndices;
	//vertex data
	vector < float > vertices;
	vector < float > normals;
	vector < float > textureCoords;

	unsigned int vertexCount = 0;

	ifstream file(filename);
	string str;
	if (!file) throw _STDEXCEPT_;

	while (getline(file, str)) {
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
				cout << "Invalid OBJ format" << endl;

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

	cout << "Parsed obj successfully." << endl;

	//Code to sort attributes according to indices (not fully working, to be used with glDrawArrays())
	/*
	int indicesSize = indices.size();
	vector < float > normalsOut;
	vector < float > textureCoordsOut;
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

	cout << "UVs and Normals sorted successfully" << endl;

	//load texture into opengl
	unsigned int textureId = loadTexture(textureFile);
	unsigned int textureNormalId = -1;
	if(textureNormalFile != nullptr)
		textureNormalId = loadTexture(textureNormalFile);
	//save Id's to Entity
	Entity *newEntity = new Entity(vertices, indices, normals, textureCoords, &modelMatrix, textureId, textureNormalId);

	cout << "Loaded Entity successfully" << endl;

	cacheEntity(newEntity, filename);

	return newEntity;
}

Entity* readOBJ_better(const char* filename, const char* textureFile, const char* textureNormalFile, glm::mat4 modelMatrix) {

	//if loaded obj before, don't load again!
	if (cache.count(filename)) {
		//cout << "Cache Hit!...:  " << filename << endl;
		vector<unsigned int> ids(cache.at(filename));
		Entity* cachedEntity = new Entity();
		cachedEntity->loadCached(ids[0], ids[1], ids[2], ids[3], ids[4], ids[5], ids[6], &modelMatrix);

		return cachedEntity;
	}

	tinyobj::attrib_t attrib;
	vector<tinyobj::shape_t> shapes;
	vector<tinyobj::material_t> materials;
	string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {
		throw runtime_error(warn + err);
	}

	vector<Vertex> vertices;
	vector<unsigned int> indices;

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};
			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				//attrib.texcoords[2 * index.texcoord_index + 0],
				//attrib.texcoords[2 * index.texcoord_index + 1]
			};
			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}
	vector < float > verticesOut;
	vector < float > normalsOut;
	vector < float > textureCoordsOut;

	for (Vertex v : vertices) {
		verticesOut.push_back(v.pos.x);
		verticesOut.push_back(v.pos.y);
		verticesOut.push_back(v.pos.z);

		normalsOut.push_back(v.normal.x);
		normalsOut.push_back(v.normal.y);
		normalsOut.push_back(v.normal.z);

		textureCoordsOut.push_back(v.texCoord.x);
		textureCoordsOut.push_back(v.texCoord.y);
	}


	//load diffuse texture into opengl
	unsigned int textureId = -1;
	if (textureFile != nullptr)
		textureId = loadTexture(textureFile);
	//load normal/bump texture into opengl
	unsigned int textureNormalId = -1;
	if(textureNormalFile != nullptr)
		textureNormalId = loadTexture(textureNormalFile);
	//save Id's to Entity
	Entity* newEntity = new Entity(verticesOut, indices, normalsOut, textureCoordsOut, &modelMatrix, textureId, textureNormalId);

	cacheEntity(newEntity, filename);

	return newEntity;
}

void cacheEntity(Entity* newEntity, const char* filename) {
	vector<unsigned int> idList{
		newEntity->VAO, newEntity->VBOs.at(0),
		newEntity->VBOs.at(1), newEntity->VBOs.at(2),
		newEntity->indexBufferSize,
		newEntity->textureId, newEntity->normalTextureId,
	};
	cache.insert(pair<string, vector<unsigned int>>(filename, idList));
}

string readShader(const char* filename) {
	ifstream ifs(filename);
	return string((istreambuf_iterator<char>(ifs)),
		(istreambuf_iterator<char>()));
}

/* TERRAIN */

void genTerrain(const char* heightMapFile, vector<string> textures , glm::mat4 modelMatrix, Terrain* newTerrain, bool flat) {

	//read heightMapFile
	int width, height, nrChannels;
	unsigned char* data = stbi_load(heightMapFile, &width, &height, &nrChannels, STBI_grey);
	if (!data)
		cout << "Failed to load Heightmap!" << endl;

	int VERTEX_COUNT = height; //assuming square image
	float MAX_PIXEL_COLOUR = 256 * 256 * 256;

	//generate vertices, normals uvs
	vector<float> vertices(VERTEX_COUNT * VERTEX_COUNT * 3);
	vector<float> normals(VERTEX_COUNT * VERTEX_COUNT * 3);
	vector<float> uvs(VERTEX_COUNT * VERTEX_COUNT * 2);

	vector<vector<float>> heights;
	heights.resize(VERTEX_COUNT, vector<float>(VERTEX_COUNT, 0));

	int vertexPointer = 0;
	for (int i = 0; i < VERTEX_COUNT; i++) {
		for (int j = 0; j < VERTEX_COUNT; j++) {
			//vertices
			float vertHeight = getHeight(i, j, data, height, nrChannels, newTerrain->MAX_HEIGHT);
			heights[i][j] = flat ? 0 : vertHeight; //add height to collision buffer
			vertices[vertexPointer * 3] = (float)j / ((float)VERTEX_COUNT - 1) * newTerrain->mapSize;
			vertices[vertexPointer * 3 + 1] = flat ? 0 : vertHeight;
			vertices[vertexPointer * 3 + 2] = (float)i / ((float)VERTEX_COUNT - 1) * newTerrain->mapSize;
			//normals
			glm::vec3 normal = glm::vec3(0, 1, 0);//calculateNormal(j, i, image);
			normal = flat ? glm::vec3(0, 1, 0) : calculateNormal(i, j, data, height, nrChannels, newTerrain->MAX_HEIGHT);
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
	vector<unsigned int> indices(VERTEX_COUNT * VERTEX_COUNT * 6);
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
	vector<int> textureIds(5);
	for (unsigned int i = 0; i < textures.size(); i++)
		textureIds[i] = loadTexture(textures.at(i).c_str());

	//load Terrain with data
	newTerrain->load(vertices, indices, normals, uvs, modelMatrix, textureIds, heights);

	stbi_image_free(data);
}

glm::vec3 calculateNormal(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT) {
	float heightD = getHeight(i - 1, j, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightU = getHeight(i + 1, j, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightL = getHeight(i, j - 1, heightMap, height, nrChannels, MAX_HEIGHT);
	float heightR = getHeight(i, j + 1, heightMap, height, nrChannels, MAX_HEIGHT);
	glm::vec3 normal = glm::vec3(heightL - heightR, 2.0f, heightD - heightU);
	return glm::normalize(normal);
}

float getHeight(int i, int j, unsigned char* heightMap, int height, int nrChannels, float MAX_HEIGHT) {
	if (i < 0 || i >= height || j < 0 || j >= height) return NULL;

	unsigned char* pixelOffset = heightMap + (i * height + j);// *nrChannels;
	unsigned char r = pixelOffset[0];
	unsigned char g = pixelOffset[1];
	unsigned char b = pixelOffset[2];
	float currentHeight = ((r + g + b) / 3.0f) / 255.0f;

	return currentHeight * MAX_HEIGHT;
}