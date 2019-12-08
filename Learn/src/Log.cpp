#include "Log.h"


Log::Log()
{
	using namespace std;
	streambuf* coutBuf = cout.rdbuf();

	of = new ofstream("out.txt");
	streambuf* fileBuf = of->rdbuf();
	cout.rdbuf(fileBuf);
}

Log * Log::Instance()
{
	static Log instance;
	return &instance;
}


Log::~Log()
{
	of->flush();
	of->close();
	of = nullptr;
}

void Log::Init()
{
	Log::Instance();
}
