#include "Utility.h"
#include "stb_image.h"

bool CheckError(std::string info)
{
	bool result = false;
	for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
	{
		result = true;
		COUT<< info << "Error:" << err << std::endl;
	}
	std::cout.flush();
	return result;
}


namespace
{
	shared_ptr<Mesh> GlobalCubeMesh;
}

shared_ptr<Mesh> GetCubeMesh()
{
	if (GlobalCubeMesh) return GlobalCubeMesh;

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
		0.5f,  0.5f, 0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   0.0f, 0.0f,
		-0.5f,  0.5f,  -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	vector<Vertex> vertexList;
	for (size_t i = 0; i < sizeof(vertices) / sizeof(float);)
	{
		Vertex vertex;
		vertex.Position.x = vertices[i++];
		vertex.Position.y = vertices[i++];
		vertex.Position.z = vertices[i++];

		vertex.Normal.x = vertices[i++];
		vertex.Normal.y = vertices[i++];
		vertex.Normal.z = vertices[i++];

		vertex.TexCoords.x = vertices[i++];
		vertex.TexCoords.y = vertices[i++];

		vertexList.push_back(vertex);
	}

	vector<unsigned int> indices = {
	0,1,2,3,4,5,6,7,8,9,
	10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,26,27,28,29,
	30,31,32,33,34,35
	};

	vector<TextureSlot> textures;
	GlobalCubeMesh = make_shared<Mesh>(vertexList, indices, textures);

	return GlobalCubeMesh;
}

Mesh GetPlanMesh(int sizex, int sizez, int gridCountX, int gridCountZ)
{
	vector<unsigned int> indices;
	vector<Vertex> vertexList;

	float offsetX = -sizex / 2.0f;
	float offsetZ = -sizez / 2.0f;

	float itemSizeX = sizex / (float)gridCountX;
	float itemSizeZ = sizez / (float)gridCountZ;
	for (size_t zIndex = 0; zIndex <= gridCountZ; zIndex++)
	{
		for (size_t xIndex = 0; xIndex <= gridCountX; xIndex++)
		{

			Vertex vertex;
			vertex.Position.z = zIndex * itemSizeZ + offsetZ;
			vertex.Position.x = xIndex * itemSizeX + offsetX;
			vertex.Position.y = 0.0f;
			
			vertex.TexCoords.x = xIndex / (float)gridCountX;
			vertex.TexCoords.y = zIndex / (float)gridCountZ;

			vertex.Normal = glm::vec3(0, 1, 0);
			vertexList.push_back(vertex);
		}
	}

	for (size_t zIndex = 0; zIndex < gridCountZ; zIndex++)
	{
		for (size_t xIndex = 0; xIndex < gridCountX; xIndex++)
		{
			// D---C
			// |   |
			// B---A
			int A = zIndex * (gridCountZ+1) + xIndex;
			int B = zIndex * (gridCountZ + 1) + xIndex + 1;
			int C = (zIndex+1) * (gridCountZ + 1) + xIndex;
			int D = (zIndex+1) * (gridCountZ + 1) + xIndex+1;

			indices.push_back(A);
			indices.push_back(C);
			indices.push_back(D);
			indices.push_back(D);
			indices.push_back(B);
			indices.push_back(A);
		}
	}


	vector<TextureSlot> textures;
	Mesh mesh(vertexList, indices, textures);

	return mesh;
}

Mesh GetScreenMesh()
{
	float vertices[] = {
		-1.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,	 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,  0.0f,  0.0f, -1.0f,   1.0f, 1.0f,
		-1.0f, -1.0f, 0.0f,  0.0f,  0.0f, -1.0f,   0.0f, 0.0f,
		1.0f, -1.0f, 0.0f,  0.0f,  0.0f, -1.0f,   1.0f, 0.0f,
		
	};

	vector<Vertex> vertexList;
	for (size_t i = 0; i < sizeof(vertices) / sizeof(float);)
	{
		Vertex vertex;
		vertex.Position.x = vertices[i++];
		vertex.Position.y = vertices[i++];
		vertex.Position.z = vertices[i++];

		vertex.Normal.x = vertices[i++];
		vertex.Normal.y = vertices[i++];
		vertex.Normal.z = vertices[i++];

		vertex.TexCoords.x = vertices[i++];
		vertex.TexCoords.y = vertices[i++];

		vertexList.push_back(vertex);
	}

	vector<unsigned int> indices = {3, 1, 0, 0, 2, 3};
	vector<TextureSlot> textures;
	Mesh screen(vertexList, indices, textures);
	return screen;
}

void LoadCubeTexture(unsigned int &textureID, vector<string> faces)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}