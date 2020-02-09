#include "Utility.h"

bool CheckError(std::string info)
{
	bool result = false;
	for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
	{
		result = true;
		COUT<< info << "Error:" << err << std::endl;
	}
	std::cout.flush();
	return result;
}