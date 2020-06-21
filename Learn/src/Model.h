#pragma once
#include "Shader.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
class Model
{
public:
	/*  ����   */
	Model(string path)
	{
		loadModel(path);
	}
	void Draw(Shader shader, bool useMeshTexture = true);

	/*  ģ������  */
	vector<Mesh> meshes;
private:
	
	string directory;
	vector<TextureSlot> textures_loaded;


	/*  ����   */
	void loadModel(string path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	vector<TextureSlot> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		string typeName);

};
