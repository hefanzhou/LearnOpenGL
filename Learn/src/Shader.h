#pragma once
#include <string>
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Shader
{
public:
	// 程序ID
	unsigned int ID;

	// 构造器读取并构建着色器
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath);
	// 使用/激活程序
	void use();
	// uniform工具函数
	void setBool(const std::string &name, bool value) const;
	void setInt(const std::string &name, int value) const;
	void setFloat(const std::string &name, float value) const;
	void SetMatrix(const std::string &name, glm::mat4 value)  const;
	void SetVec3(const std::string &name, glm::vec3 value) const;
	void SetVec2(const std::string &name, glm::vec2 value) const;
	void SetVec4(const std::string &name, glm::vec4 value) const;
	void SetTexture(int index, const std::string &name, unsigned int textureID, unsigned int textureSlotType = GL_TEXTURE_2D);
	void SetUniformBlockBinding(const std::string &name, int bindingPoint);
};