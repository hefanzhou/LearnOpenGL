#include "Shader.h"
#include <fstream>
#include <sstream>
#include "Log.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath, const GLchar* geometryPath)
{
	shaderPath = vertexPath;

	// 1. 从文件路径中获取顶点/片段着色器
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// 保证ifstream对象可以抛出异常：
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// 打开文件
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// 读取文件的缓冲内容到数据流中
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// 关闭文件处理器
		vShaderFile.close();
		fShaderFile.close();
		// 转换数据流到string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		if (geometryPath != nullptr)
		{
			gShaderFile.open(geometryPath);
			std::stringstream gShaderStream;
			gShaderStream << gShaderFile.rdbuf();
			gShaderFile.close();
			geometryCode = gShaderStream.str();
		}

	}
	catch (std::ifstream::failure e)
	{
		COUT << vertexPath << " ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();

	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	glShaderSource(vertexShader, 1, &vShaderCode, NULL);
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		COUT << vertexPath << " ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}


	//片段着色器
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		COUT << fragmentPath << " ERROR::SHADER:Fragment::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//几何着色器
	unsigned int geometryShader = 0;
	if(geometryPath)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &gShaderCode, NULL);
		glCompileShader(geometryShader);
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			COUT << geometryPath << " ERROR::SHADER:Geometry::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
	}
	//链接
	ID = glCreateProgram();

	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	if(geometryShader > 0)
	{
		glAttachShader(ID, geometryShader);
	}
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		COUT << fragmentPath << " ERROR::SHADER::Link FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	glDeleteShader(geometryShader);
}

void Shader::use()
{
	glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{
	glUniform1i(GetUniformLocation(name), (int)value);
}
void Shader::setInt(const std::string &name, int value) const
{
	glUniform1i(GetUniformLocation(name), value);
}
void Shader::setFloat(const std::string &name, float value) const
{
	glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetMatrix(const std::string & name, glm::mat4 value) const
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string & name, glm::vec3 value) const
{
	glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void Shader::SetVec2(const std::string & name, glm::vec2 value) const
{
	glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void Shader::SetVec4(const std::string & name, glm::vec4 value) const
{
	glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void Shader::SetTexture(int index, const std::string & name, unsigned int textureID, unsigned int textureSlotType)
{
	glActiveTexture(GL_TEXTURE0 + index);
	setInt(name, index);
	glBindTexture(textureSlotType, textureID);
	glActiveTexture(GL_TEXTURE0);
}

void Shader::SetUniformBlockBinding(const std::string & name, int bindingPoint)
{
	unsigned int uniformBlockIndex = glGetUniformBlockIndex(ID, name.c_str());
	glUniformBlockBinding(ID, uniformBlockIndex, bindingPoint);
}

int Shader::GetUniformLocation(const std::string & name) const
{
	int value = glGetUniformLocation(ID, name.c_str());
	if (value < 0)
		COUT << shaderPath << ":GetUniformLocationError:" << name << std::endl;
	return value;
}
