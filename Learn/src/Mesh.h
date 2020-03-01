#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include "Shader.h"
#include <assimp/types.h>

using namespace std;
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct TextureSlot {
	TextureSlot() {};
	TextureSlot(unsigned int _id, const string& _name):id(_id), name(_name) {};
	unsigned int id;
	string name;
	aiString path;
};
class Mesh
{
public:
	/*  函数  */
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<TextureSlot> textures);
	void Draw(Shader shader);
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
private:
	unsigned int VAO, VBO, EBO;

	/*  网格数据  */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<TextureSlot> textures;

	/*  函数  */
	void setupMesh();
};

