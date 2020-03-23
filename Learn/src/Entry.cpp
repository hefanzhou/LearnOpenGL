#include "FirstMain.h"
#include "LightMain.h"
#include "ModelMain.h"
#include "Test.h"
#include "BlendMain.h"
#include "FrameBufferMain.h"
#include "MainCube.h"
#include "MainGeometryShader.h"
#include "MainInstanceDraw.h"
#include "MainShadow.h"
#include "MainMoreTexture.h"
#include "MainBloom.h"
#include "MainDeferred.h"
int main()
{
	MainDeferred::main();
	return 0;
}