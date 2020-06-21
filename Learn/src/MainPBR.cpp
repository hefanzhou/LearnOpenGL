#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "Log.h"
#include "Shader.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"
#include "Texture.h"
#include "Mesh.h"
#include "Model.h"
#include "Utility.h"
using namespace glm;
namespace MainPBR
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow *window);
	void UpdateTime();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void GenCubeMapBuffer(unsigned int &captureFBO, unsigned int &captureRBO, unsigned int &envCubemap, int width, int height);
	void RenderToCubeMapSideBySide(const unsigned int captureFBO, const unsigned int cubeMapId, int width, int height, Shader &shader);
	void GenerateSpecualBRDFLutTexture(unsigned int &brdfLUTTexture, int resourlution, unsigned int captureFBO, unsigned int captureRBO);

	void RenderPrefiterCubeMap(const unsigned int captureFBO, 
		const unsigned int captureRBO, 
		const unsigned int envCubemap, 
		const unsigned int cubeMapId, 
		int resolution);

	Camera *camera = nullptr;
	float deltaTime = 0.0f; // 当前帧与上一帧的时间差
	float lastFrame = 0.0f; // 上一帧的时间
	float pai = 3.14159f;
	int screenWidth = 800;
	int screenHeight = 800;

	int envCubeMapWidth = 512;
	int envCubeMapHeight = 512;

	int main()
	{
		// 初始化
		Log::Init();

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



		GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);

		if (window == NULL)
		{
			COUT << "Failed to create GLFW window/n";
			glfwTerminate();
			return -1;
		}
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			COUT << "Failed to initialize GLAD/n";
			return -1;
		}


		glViewport(0, 0, screenWidth, screenHeight);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);
		camera = new Camera(glm::vec3(0, 0, -30), 0, 0, 45.0f, (float)screenWidth / screenHeight);

		Shader TextureShader("./shader/PBR/simple.vs", "./shader/PBR/simple.fs");
		Shader pbrShader("./shader/PBR/PBRCommon.vs", "./shader/PBR/PBRCommon.fs");
		Shader pbrModelShader("./shader/PBR/PBRCommonFBX.vs", "./shader/PBR/PBRCommonFBX.fs");
		Shader visualNormalShader("./shader/Geometry/VisualizationNormal.vs", "./shader/Geometry/VisualizationNormal.fs", "./shader/Geometry/VisualizationNormal.gs");
		Shader rectFullImageShdaer("./shader/PBR/RectFullImage.vs", "./shader/PBR/RectFullImage.fs");
		Shader cubeMapShader("./shader/CubeMap/CubeMap.vs", "./shader/CubeMap/CubeMap.fs");

		Model gunModel("./res/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
		Texture albedoTex("res/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga", false);
		Texture metallicTex("res/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga", false);
		Texture roughnessTex("res/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga", false);
		Texture normalTex("res/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga", false);

		auto cubeMesh = GetCubeMesh();
		auto sphereMesh = GetSphereMesh();
		auto planMesh = GetPlanMesh(10, 10, 2, 2);

		unsigned int envRectTextureID = 0;
		LoadHDRTexture(envRectTextureID, "./res/IBLTexture/Lobby-Center_2k.hdr");
		//LoadHDRTexture(envRectTextureID, "./res/IBLTexture/Malibu_Overlook_3k.hdr");

		unsigned int envCubemap = 0;
		// 矩形图到立方体贴图
		{
			
			Shader rectToCubeMapShdaer("./shader/PBR/RectToCubeMap.vs", "./shader/PBR/RectToCubeMap.fs");
			unsigned int captureFBO = 0;
			unsigned int captureRBO = 0;
			GenCubeMapBuffer(captureFBO, captureRBO, envCubemap, envCubeMapWidth, envCubeMapHeight);
			rectToCubeMapShdaer.use();
			rectToCubeMapShdaer.SetTexture(0, "equirectangularMap", envRectTextureID);
			RenderToCubeMapSideBySide(captureFBO, envCubemap, envCubeMapWidth, envCubeMapHeight, rectToCubeMapShdaer);
		}

		// 漫反射IBL
		unsigned int IBLDiffuseCubeMapId = 0;
		{
			Shader GenIBLDiffuseShdaer("./shader/PBR/GenIBLDiffuse.vs", "./shader/PBR/GenIBLDiffuse.fs");
			unsigned int captureFBO = 0;
			unsigned int captureRBO = 0;
			int resolution = 32;
			GenCubeMapBuffer(captureFBO, captureRBO, IBLDiffuseCubeMapId, resolution, resolution);
			GenIBLDiffuseShdaer.use();
			GenIBLDiffuseShdaer.SetTexture(0, "irradianceMap", envCubemap, GL_TEXTURE_CUBE_MAP);

			RenderToCubeMapSideBySide(captureFBO, IBLDiffuseCubeMapId, resolution, resolution, GenIBLDiffuseShdaer);
		}

		//镜面反射IBL部分1
		unsigned int IBLSpecualCubeMapId = 0;
		{
			int resolution = 128;
			unsigned int captureFBO = 0;
			unsigned int captureRBO = 0;
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
			GenCubeMapBuffer(captureFBO, captureRBO, IBLSpecualCubeMapId, resolution, resolution);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
			RenderPrefiterCubeMap(captureFBO, captureRBO, envCubemap, IBLSpecualCubeMapId, resolution);
		}

		//镜面反射部分2
		unsigned int IBLSpecualBRDFLutTextureId = 0;
		{
			int resolution = 512;
			unsigned int captureFBO = 0;
			unsigned int captureRBO = 0;
			glGenFramebuffers(1, &captureFBO);
			glGenRenderbuffers(1, &captureRBO);

			GenerateSpecualBRDFLutTexture(IBLSpecualBRDFLutTextureId, resolution, captureFBO, captureRBO);

		}

		std::cout.flush();
		glm::vec3 lightPositions[] = {
			 glm::vec3(-10.0f,  10.0f, -5.0f),
			 glm::vec3(10.0f,  10.0f, -5.0f),
			 glm::vec3(-10.0f, -10.0f, -5.0f),
			 glm::vec3(10.0f, -10.0f, -5.0f),
		};
		glm::vec3 lightColors[] = {
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f),
			glm::vec3(300.0f, 300.0f, 300.0f)
		};

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glViewport(0, 0, screenWidth, screenHeight);

		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // don't forget to clear the stencil buffer!
			glm::mat4 PVTrans = camera->GetPVMatrix();

			//全景图
			if(true)
			{
				auto CameraViewNoTranslate = glm::mat4(glm::mat3(camera->ViewMatrix));
				cubeMapShader.use();
				cubeMapShader.SetMatrix("transformVP", camera->PerspectiveMatrix*CameraViewNoTranslate);
				cubeMapShader.SetTexture(0, "texture_diffuse", envCubemap, GL_TEXTURE_CUBE_MAP);
				cubeMesh->Draw(cubeMapShader);
			}

			// debug texture
			if(false)
			{
				TextureShader.use();
				mat4 model;
				model = translate(model, vec3(0, 0, 0));
				model = glm::rotate(model, -90.0f, glm::vec3(1, 0, 0));
				TextureShader.SetMatrix("transformMVP", PVTrans*model);
				TextureShader.SetTexture(0, "diffsuleTexture", IBLSpecualBRDFLutTextureId);
				planMesh.Draw(TextureShader);
			}

			//PBR
			if(false)
			{
				int count = 10;
				float offset = 2.1;
				pbrShader.use();
				pbrShader.SetVec3("camPos", camera->Position);
				pbrShader.SetVec3("albedo", vec3(0.4f, 0.4f, 0.4f));
				pbrShader.setFloat("ao", 1);

				pbrShader.SetTexture(0, "irradianceMap", IBLDiffuseCubeMapId, GL_TEXTURE_CUBE_MAP);
				pbrShader.SetTexture(1, "prefilterMap", IBLSpecualCubeMapId, GL_TEXTURE_CUBE_MAP);
				pbrShader.SetTexture(2, "brdfLUT", IBLSpecualBRDFLutTextureId);

				char strBuffer[50];
				for (size_t i = 0; i < 4; i++)
				{
					sprintf_s(strBuffer, "lightPositions[%d]", i);
					pbrShader.SetVec3(strBuffer, lightPositions[i]);
					sprintf_s(strBuffer, "lightColors[%d]", i);
					pbrShader.SetVec3(strBuffer, lightColors[i]);
				}
				for (int i = 0; i < count; i++)
				{
					pbrShader.setFloat("metallic", (float)i / glm::max(count - 1, 1));
					for (int j = 0; j < count; j++)
					{
						mat4 model;
						model = translate(model, vec3((i-count/2)*offset, (j - count / 2)*offset, 0));
						pbrShader.SetMatrix("transformMVP", PVTrans*model);
						pbrShader.SetMatrix("transformM", model);
						pbrShader.setFloat("roughness", (float)j / glm::max(count - 1, 1));
						sphereMesh->Draw(pbrShader);
					}
				}
			}


			//PBR Model
			if (true)
			{
				pbrModelShader.use();
				pbrModelShader.SetVec3("camPos", camera->Position);
				pbrModelShader.setFloat("ao", 1);

				pbrModelShader.SetTexture(0, "irradianceMap", IBLDiffuseCubeMapId, GL_TEXTURE_CUBE_MAP);
				pbrModelShader.SetTexture(1, "prefilterMap", IBLSpecualCubeMapId, GL_TEXTURE_CUBE_MAP);
				pbrModelShader.SetTexture(2, "brdfLUT", IBLSpecualBRDFLutTextureId);
				pbrModelShader.SetTexture(3, "albedoTexture", albedoTex.GetTextureID());
				pbrModelShader.SetTexture(4, "metallicTexture", metallicTex.GetTextureID());
				pbrModelShader.SetTexture(5, "roughnessTexture", roughnessTex.GetTextureID());
				pbrModelShader.SetTexture(6, "normalTexture", normalTex.GetTextureID());

				char strBuffer[50];
				for (size_t i = 0; i < 4; i++)
				{
					sprintf_s(strBuffer, "lightPositions[%d]", i);
					pbrModelShader.SetVec3(strBuffer, lightPositions[i]);
					sprintf_s(strBuffer, "lightColors[%d]", i);
					pbrModelShader.SetVec3(strBuffer, lightColors[i]);
				}

				mat4 modelTrans;
				modelTrans = translate(modelTrans, vec3(10, 0, 0));
				modelTrans = glm::scale(modelTrans, vec3(0.3, 0.3, 0.3));
				modelTrans = rotate(modelTrans, -pai / 2, vec3(0, 1, 0));
				modelTrans = rotate(modelTrans, -pai /2, vec3(1, 0, 0));
				pbrModelShader.SetMatrix("transformMVP", PVTrans*modelTrans);
				pbrModelShader.SetMatrix("transformM", modelTrans);
				gunModel.Draw(pbrModelShader, false);
			}
			glfwSwapBuffers(window);
			glfwPollEvents();

			if (CheckError()) break;
		}

		glfwTerminate();
		return 0;
	}



	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void processInput(GLFWwindow *window)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera->ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera->ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera->ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera->ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			camera->ProcessKeyboard(Camera_Movement::UP, deltaTime);
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			camera->ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
	}


	void UpdateTime()
	{
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}

	bool firstMouse = true;
	double lastX;
	double lastY;
	void mouse_callback(GLFWwindow * window, double xpos, double ypos)
	{
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		double xoffset = lastX - xpos;
		double yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		double sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		if (glfwGetMouseButton(window, 0) == GLFW_PRESS)
		{
			camera->ProcessMouseMovement((float)xoffset, (float)yoffset);
		}

	}

	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		camera->ProcessMouseScroll((float)yoffset);
	}

	void GenCubeMapBuffer(unsigned int &captureFBO, unsigned int &captureRBO, unsigned int &envCubemap, int width, int height)
	{
		
		glGenFramebuffers(1, &captureFBO);
		glGenRenderbuffers(1, &captureRBO);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

		glGenTextures(1, &envCubemap);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		for (unsigned int i = 0; i < 6; ++i)
		{
			// note that we store each face with 16 bit floating point values
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
				width, height, 0, GL_RGB, GL_FLOAT, nullptr);
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	void RenderToCubeMapSideBySide(const unsigned int captureFBO,  const unsigned int cubeMapId, int width, int height, Shader &shader)
	{
		auto cube = GetCubeMesh();
		shader.SetMatrix("projection", captureProjection);

		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		for (unsigned int i = 0; i < 6; ++i)
		{
			shader.SetMatrix("view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapId, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			cube->Draw(shader);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RenderPrefiterCubeMap(const unsigned int captureFBO, const unsigned int captureRBO, const unsigned int envCubemap, const unsigned int cubeMapId, int resolution)
	{
		auto cube = GetCubeMesh();
		Shader IBLPrefilterShdaer("./shader/PBR/IBLPrefilter.vs", "./shader/PBR/IBLPrefilter.fs");
		IBLPrefilterShdaer.use();
		IBLPrefilterShdaer.SetMatrix("projection", captureProjection);
		IBLPrefilterShdaer.SetTexture(0, "environmentMap", envCubemap, GL_TEXTURE_CUBE_MAP);
		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

		unsigned int maxMipLevels = 5;
		for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
		{
			// reisze framebuffer according to mip-level size.
			unsigned int mipResolution = resolution * std::pow(0.5, mip);
			glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipResolution, mipResolution);
			glViewport(0, 0, mipResolution, mipResolution);

			float roughness = (float)mip / (float)(maxMipLevels - 1);
			IBLPrefilterShdaer.setFloat("roughness", roughness);
			for (unsigned int i = 0; i < 6; ++i)
			{
				IBLPrefilterShdaer.SetMatrix("view", captureViews[i]);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubeMapId, mip);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				cube->Draw(IBLPrefilterShdaer);
			}
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GenerateSpecualBRDFLutTexture(unsigned int &brdfLUTTexture, int resourlution, unsigned int captureFBO, unsigned int captureRBO)
	{
		glGenTextures(1, &brdfLUTTexture);
		// pre-allocate enough memory for the LUT texture.
		glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, resourlution, resourlution, 0, GL_RG, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resourlution, resourlution);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

		glViewport(0, 0, resourlution, resourlution);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Shader shader("./shader/PBR/IBLReflectBRDF.vs", "./shader/PBR/IBLReflectBRDF.fs");
		auto screenMesh = GetPlanMesh(2, 2, 1, 1);

		shader.use();
		screenMesh.Draw(shader);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}