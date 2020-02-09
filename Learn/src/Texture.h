#pragma once
#include <string>
using namespace std;

class Texture
{
public:
	Texture(string texturePath, bool alpha);
	~Texture();

	unsigned int GetTextureID()
	{
		return textureId;
	}
private:
	unsigned int textureId = 0;
	string texturePath;
};

