#pragma once

#include "glad/glad.h"
#include "Log.h"
#include <string>
#include "Mesh.h"

bool CheckError(std::string info = "");
shared_ptr<Mesh> GetCubeMesh();
Mesh GetPlanMesh(int sizex, int sizez, int gridCountX, int gridCountZ);
Mesh GetScreenMesh();
void LoadCubeTexture(unsigned int &textureID, vector<string> faces);
void LoadHDRTexture(unsigned int &textureID, const string &path);
shared_ptr<Mesh> GetSphereMesh();

template<typename T, typename T2>
void CopyDiffTypeVec3To(T &vec1, T2 &vec2)
{
	vec2.x = vec1.x;
	vec2.y = vec1.y;
	vec2.z = vec1.z;
}
