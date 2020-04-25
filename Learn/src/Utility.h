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

