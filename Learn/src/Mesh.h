#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include "Shader.h"
#include <assimp/types.h>
#include <assimp/material.h>
using namespace std;
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Tangent;
	glm::vec3 Bitangent;
};


struct TextureSlot {
	TextureSlot() {};
	TextureSlot(unsigned int _id, const string& _name):id(_id), name(_name) {};
	unsigned int id;
	string name;
	aiString path;
	aiTextureType textureType;
};
class Mesh
{
public:
	/*  ����  */
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<TextureSlot> textures);
	void Draw(Shader shader);
	void DrawWithoutTexture(Shader shader);
	Mesh() = delete;

	void SetTexture(int index, const TextureSlot &slot)
	{
		if (textures.size() < index + 1)
			textures.resize(index + 1);

		textures[index] = slot;
	}

	void BindVertexArray()
	{
		glBindVertexArray(VAO);
	}

	int IndecesSize()
	{
		return indices.size();
	}

	void EnableTangent(bool enable)
	{
		enableTangent = enable;
	}
	/*  ����  */
	void setupMesh();

private:
	unsigned int VAO, VBO, EBO;
	bool enableTangent;

	/*  ��������  */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<TextureSlot> textures;

};

