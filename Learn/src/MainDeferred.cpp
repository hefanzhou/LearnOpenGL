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
#include<random>
#include "MainGeometryShader.h"

namespace MainDeferred
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow *window);
	void UpdateTime();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void GenFreamBuffer(GLuint &gBuffer, GLuint &gPosition, GLuint &gNormal, GLuint &gAlbedoSpec, int scr_width, int scr_height);
	void GenSSAOKernel(std::vector<glm::vec3> &ssaoKernel);
	void GenRotateNoiseTexture(GLuint &noiseTexture);
	void GenSSAOBuffer(GLuint &ssaoFBO, GLuint &ssaoColorBuffer, int scr_width, int scr_height);


	Camera *camera = nullptr;
	float deltaTime = 0.0f; // 当前帧与上一帧的时间差
	float lastFrame = 0.0f; // 上一帧的时间
	float pai = 3.14159f;
	int screenWidth = 800;
	int screenHeight = 800;

	GLFWwindow* window = nullptr;
	int mainDeferred()
	{
		// 初始化
		Log::Init();

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
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

		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		Shader screenShader("./shader/Deferred/Screen.vs", "./shader/Deferred/Screen.fs");
		Shader commonShader("./shader/Deferred/commonLight.vs", "./shader/Deferred/commonLight.fs");
		Shader DefferdShadingShader("./shader/Deferred/DefferdShading.vs", "./shader/Deferred/DefferdShading.fs");

		camera = new Camera(glm::vec3(0, 2, -5), 0, -20, 45.0f, (float)screenWidth / screenHeight);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		auto planMesh = GetPlanMesh(5, 5, 10, 10);
		auto screenMesh = GetPlanMesh(2, 2, 1, 1);
		auto cubeMesh = *GetCubeMesh();
		Texture diffsuleTexture("./res/bricks2.jpg", false);
		Texture diffsuleTexture2("./res/brickwall.jpg", false);

		std::vector<glm::vec3> objectPositions;
		objectPositions.push_back(glm::vec3(-3.0, 1.0, -3.0));
		objectPositions.push_back(glm::vec3(0.0, 1.0, -3.0));
		objectPositions.push_back(glm::vec3(3.0, 1.0, -3.0));
		objectPositions.push_back(glm::vec3(-3.0, 1.0, 0.0));
		objectPositions.push_back(glm::vec3(0.0, 1.0, 0.0));
		objectPositions.push_back(glm::vec3(3.0, 1.0, 0.0));
		objectPositions.push_back(glm::vec3(-3.0, 1.0, 3.0));
		objectPositions.push_back(glm::vec3(0.0, 1.0, 3.0));
		objectPositions.push_back(glm::vec3(3.0, 1.0, 3.0));
		// - Colors
		const GLuint NR_LIGHTS = 32;
		std::vector<glm::vec3> lightPositions;
		std::vector<glm::vec3> lightColors;
		srand(13);
		for (GLuint i = 0; i < NR_LIGHTS; i++)
		{
			// Calculate slightly random offsets
			GLfloat xPos = ((rand() % 100) / 100.0) * 8.0 - 4.0;
			GLfloat yPos = ((rand() % 100) / 100.0) * 6.0 - 2.0;
			GLfloat zPos = ((rand() % 100) / 100.0) * 8.0 - 5.0;
			lightPositions.push_back(glm::vec3(xPos, yPos, zPos));
			// Also calculate random color
			GLfloat rColor = ((rand() % 100) / 200.0) + 0.5; 
			GLfloat gColor = ((rand() % 100) / 200.0) + 0.5; 
			GLfloat bColor = ((rand() % 100) / 200.0) + 0.5;
			int weight = (rand() % 100);
			if (weight < 30)
				rColor = 0;
			else if (weight < 60)
				gColor = 0;
			else
				bColor = 0;

			lightColors.push_back(glm::vec3(rColor, gColor, bColor));
		}

		std::cout.flush();
		GLuint gBuffer = 0;
		GLuint gPosition = 0;
		GLuint gNormal = 0;
		GLuint gAlbedoSpec = 0;
		GenFreamBuffer(gBuffer, gPosition, gNormal, gAlbedoSpec, screenWidth, screenHeight);

		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glm::vec3 LightPos(-2, 0.8, 2);
			auto lightColor = glm::vec3(1, 1, 1);

			
			glm::mat4 PVTrans = camera->GetPVMatrix();
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//填充gBuffer
			{
				commonShader.use();
				glm::mat4 model;
				model = glm::translate(model, glm::vec3(0, 0, 0));
				commonShader.SetMatrix("transformMVP", PVTrans*model);
				commonShader.SetMatrix("transformM", model);
				commonShader.SetTexture(0, "texture_diffuse1", diffsuleTexture.GetTextureID());
				planMesh.Draw(commonShader);
				
				for (auto itemPos: objectPositions)
				{
					model = glm::mat4(1.0f);
					model = glm::translate(model, itemPos);
					commonShader.SetMatrix("transformMVP", PVTrans*model);
					commonShader.SetMatrix("transformM", model);
					commonShader.SetTexture(0, "texture_diffuse1", diffsuleTexture2.GetTextureID());
					cubeMesh.Draw(commonShader);
				}
				
			}

			//着色
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			{
				DefferdShadingShader.use();
				DefferdShadingShader.SetTexture(0, "gPosition", gPosition);
				DefferdShadingShader.SetTexture(1, "gNormal", gNormal);
				DefferdShadingShader.SetTexture(2, "gAlbedoSpec", gAlbedoSpec);
				DefferdShadingShader.SetVec3("viewPos", camera->Position);
				for (GLuint i = 0; i < lightPositions.size(); i++)
				{
					DefferdShadingShader.SetVec3(("lights[" + std::to_string(i) + "].Position").c_str(), lightPositions[i]);
					DefferdShadingShader.SetVec3(("lights[" + std::to_string(i) + "].Color").c_str(), lightColors[i]);
				}
				planMesh.Draw(DefferdShadingShader);
			}
			
			glfwSwapBuffers(window);
			glfwPollEvents();

			if (CheckError()) break;
		}
		glfwTerminate();
		return 0;
	}


	int mainSSAO()
	{
		// 初始化
		Log::Init();

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(screenWidth, screenHeight, "LearnOpenGL", NULL, NULL);
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

		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		Shader screenShader("./shader/Deferred/Screen.vs", "./shader/Deferred/Screen.fs");
		Shader commonShader("./shader/Deferred/ssaoitem.vs", "./shader/Deferred/ssaoitem.fs");
		Shader DefferdShadingShader("./shader/Deferred/DefferdShading.vs", "./shader/Deferred/DefferdShading.fs");
		Shader ssaoShader("./shader/Deferred/GenSSAO.vs", "./shader/Deferred/GenSSAO.fs");

		camera = new Camera(glm::vec3(0, 2, -5), 0, -20, 45.0f, (float)screenWidth / screenHeight);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		auto planMesh = GetPlanMesh(5, 5, 10, 10);
		auto screenMesh = GetPlanMesh(2, 2, 1, 1);
		auto cubeMesh = *GetCubeMesh();
		Texture diffsuleTexture("./res/bricks2.jpg", false);
		Texture diffsuleTexture2("./res/brickwall.jpg", false);


		std::cout.flush();
		GLuint gBuffer = 0;
		GLuint gPosition = 0;
		GLuint gNormal = 0;
		GLuint gAlbedoSpec = 0;
		GenFreamBuffer(gBuffer, gPosition, gNormal, gAlbedoSpec, screenWidth, screenHeight);
		GLuint noiseTexture = 0;
		GenRotateNoiseTexture(noiseTexture);
		GLuint ssaoBuffer = 0;
		GLuint ssaoTexture = 0;
		GenSSAOBuffer(ssaoBuffer, ssaoTexture, screenWidth, screenHeight);
		vector<glm::vec3> simples;
		GenSSAOKernel(simples);
		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glm::vec3 LightPos(-2, 0.8, 2);
			auto lightColor = glm::vec3(1, 1, 1);


			glm::mat4 PVTrans = camera->GetPVMatrix();
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//填充gBuffer
			{
				commonShader.use();
				glm::mat4 model;
				model = glm::translate(model, glm::vec3(0, 0, 0));
				commonShader.SetMatrix("transformMVP", PVTrans*model);
				commonShader.SetMatrix("transformM", model);
				commonShader.SetMatrix("transformMV", camera->ViewMatrix*model);

				planMesh.Draw(commonShader);

				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(0, 0.5f, 0));
				commonShader.SetMatrix("transformMVP", PVTrans*model);
				commonShader.SetMatrix("transformM", model);
				commonShader.SetMatrix("transformMV", camera->ViewMatrix*model);
				cubeMesh.Draw(commonShader);

			}

			//ssao
			{
				glBindFramebuffer(GL_FRAMEBUFFER, ssaoBuffer);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				ssaoShader.use();
				ssaoShader.SetTexture(0, "gPositionDepth", gPosition);
				ssaoShader.SetTexture(1, "gNormal", gNormal);
				ssaoShader.SetTexture(2, "texNoise", noiseTexture);
				char strBuffer[60];
				int index = 0;
				for (auto sample: simples)
				{
					sprintf_s(strBuffer, "samples[%d]", index);
					ssaoShader.SetVec3(strBuffer, sample);
					++index;
				}
				ssaoShader.SetMatrix("projection", camera->PerspectiveMatrix);
				screenMesh.Draw(ssaoShader);
			}

			//着色
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			if (false)
			{
				DefferdShadingShader.use();
				DefferdShadingShader.SetTexture(0, "gPosition", gPosition);
				DefferdShadingShader.SetTexture(1, "gNormal", gNormal);
				DefferdShadingShader.SetTexture(2, "gAlbedoSpec", gAlbedoSpec);
				DefferdShadingShader.SetVec3("viewPos", camera->Position);
				screenMesh.Draw(DefferdShadingShader);
			}
			else
			{
				screenShader.use();
				screenShader.SetTexture(0, "diffuseTexture", gNormal);
				screenMesh.Draw(screenShader);
			}

			glfwSwapBuffers(window);
			glfwPollEvents();

			if (CheckError()) break;
		}
		glfwTerminate();
		return 0;
	}

	int main()
	{
		mainSSAO();
		return 0;
	}


	GLfloat lerp(GLfloat a, GLfloat b, GLfloat f)
	{
		return a + f * (b - a);
	}

	void GenSSAOKernel(std::vector<glm::vec3> &ssaoKernel)
	{
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
		std::default_random_engine generator;
		
		for (GLuint i = 0; i < 64; ++i)
		{
			glm::vec3 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator)
			);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			GLfloat scale = GLfloat(i) / 64.0;
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			ssaoKernel.push_back(sample);
		}
	}

	void GenRotateNoiseTexture(GLuint &noiseTexture)
	{
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // 随机浮点数，范围0.0 - 1.0
		std::default_random_engine generator;
		std::vector<glm::vec3> ssaoNoise;
		for (GLuint i = 0; i < 16; i++)
		{
			glm::vec3 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f);
			ssaoNoise.push_back(noise);
		}

		
		glGenTextures(1, &noiseTexture);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void GenSSAOBuffer(GLuint &ssaoFBO, GLuint &ssaoColorBuffer, int scr_width, int scr_height)
	{
		glGenFramebuffers(1, &ssaoFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

		glGenTextures(1, &ssaoColorBuffer);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, scr_width, scr_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	}

	void GenFreamBuffer(GLuint &gBuffer, GLuint &gPosition, GLuint &gNormal, GLuint &gAlbedoSpec,int scr_width, int scr_height)
	{
		
		glGenFramebuffers(1, &gBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		
		// 深度
		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scr_width, scr_width);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

		// - 位置颜色缓冲
		glGenTextures(1, &gPosition);
		glBindTexture(GL_TEXTURE_2D, gPosition);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, scr_width, scr_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

			// - 法线颜色缓冲
		glGenTextures(1, &gNormal);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, scr_width, scr_height, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

		// - 颜色 + 镜面颜色缓冲
		glGenTextures(1, &gAlbedoSpec);
		glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scr_width, scr_height, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

		// - 告诉OpenGL我们将要使用(帧缓冲的)哪种颜色附件来进行渲染
		GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, attachments);
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

	
}