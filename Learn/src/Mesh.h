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
	unsigned int id;
	string type;
	aiString path;
};
class Mesh
{
public:
	/*  函数  */
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<TextureSlot> textures);
	void Draw(Shader shader);
	Mesh() = delete;
private:
	unsigned int VAO, VBO, EBO;

	/*  网格数据  */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<TextureSlot> textures;

	/*  函数  */
	void setupMesh();
};

