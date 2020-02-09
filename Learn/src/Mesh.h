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
	/*  ����  */
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<TextureSlot> textures);
	void Draw(Shader shader);
	Mesh() = delete;
private:
	unsigned int VAO, VBO, EBO;

	/*  ��������  */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<TextureSlot> textures;

	/*  ����  */
	void setupMesh();
};

