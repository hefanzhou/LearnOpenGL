#pragma once
#include "Shader.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
class Model
{
public:
	/*  函数   */
	Model(string path)
	{
		loadModel(path);
	}
	void Draw(Shader shader, bool useMeshTexture = true);

	/*  模型数据  */
	vector<Mesh> meshes;
private:
	
	string directory;
	vector<TextureSlot> textures_loaded;


	/*  函数   */
	void loadModel(string path);
	void processNode(aiNode *node, const aiScene *scene);
	Mesh processMesh(aiMesh *mesh, const aiScene *scene);
	vector<TextureSlot> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
		string typeName);

};
