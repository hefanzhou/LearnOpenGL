#pragma once

#include "glad/glad.h"
#include "Log.h"
#include <string>
#include "Mesh.h"

bool CheckError(std::string info = "");
shared_ptr<Mesh> GetCubeMesh();