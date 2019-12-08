#pragma once
#include <iostream>
#include <fstream>
#define COUT std::cout



class Log
{
public:
	~Log();
	static void Init();
private:
	Log();

	static Log* Instance();
	std::ofstream *of;
};

