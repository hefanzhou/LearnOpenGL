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
#include "MainGeometryShader.h"

namespace MainBloom
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow *window);
	void UpdateTime();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void GenFrameBuffer(GLuint &hdrFBO, GLuint *colorBuffers);
	void GenPingPongBuffer(unsigned int *pingpongFBO, unsigned int *pingpongColorbuffers);

	Camera *camera = nullptr;
	float deltaTime = 0.0f; // 当前帧与上一帧的时间差
	float lastFrame = 0.0f; // 上一帧的时间
	float pai = 3.14159f;
	int screenWidth = 800;
	int screenHeight = 800;

	int shadowWidth = 1024;
	int shadowHeight = 1024;
	GLFWwindow* window = nullptr;
	int main()
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

		Shader screenShader("./shader/Bloom/Screen.vs", "./shader/Bloom/Screen.fs");
		Shader commonShader("./shader/Bloom/commonLight.vs", "./shader/Bloom/commonLight.fs");
		Shader ColorShader("./shader/Bloom/Light.vs", "./shader/Bloom/Light.fs");
		Shader BlurShader("./shader/Bloom/Screen.vs", "./shader/Bloom/BlurScreen.fs");
		Shader BloomShader("./shader/Bloom/Screen.vs", "./shader/Bloom/Bloom.fs");

		camera = new Camera(glm::vec3(0, 2, -5), 0, -20, 45.0f, (float)screenWidth / screenHeight);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.f, 0.f, 0.f, 1.0f);

		auto planMesh = GetPlanMesh(5, 5, 2, 2);
		auto screenMesh = GetPlanMesh(2, 2, 1, 1);
		auto cubeMesh = *GetCubeMesh();
		Texture diffsuleTexture("./res/bricks2.jpg", false);
		Texture diffsuleTexture2("./res/brickwall.jpg", false);

		std::cout.flush();
		GLuint hdrFBO = 0;
		GLuint colorBuffers[2];
		GenFrameBuffer(hdrFBO, colorBuffers);
		
		unsigned int pingpongFBO[2], pingpongColorbuffers[2];
		GenPingPongBuffer(pingpongFBO, pingpongColorbuffers);

		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glm::vec3 LightPos(-2, 0.8, 2);
			auto lightColor = glm::vec3(1, 1, 1);

			
			glm::mat4 PVTrans = camera->GetPVMatrix();
			glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			{
				commonShader.use();
				glm::mat4 model;
				model = glm::translate(model, glm::vec3(0, 0, 0));
				commonShader.SetMatrix("transformMVP", PVTrans*model);
				commonShader.SetMatrix("transformM", model);
				commonShader.SetTexture(0, "diffuseTexture", diffsuleTexture.GetTextureID());
				commonShader.SetVec3("lightPos", LightPos);
				commonShader.SetVec3("lightColor", glm::vec3(1, 1, 1));
				commonShader.SetVec3("viewPos", camera->Position);
				planMesh.Draw(commonShader);
			}

			{
				glm::mat4 model;
				model = glm::translate(model, glm::vec3(0, 0.5, 0));
				commonShader.SetMatrix("transformMVP", PVTrans*model);
				commonShader.SetTexture(0, "diffuseTexture", diffsuleTexture2.GetTextureID());
				cubeMesh.Draw(commonShader);
			}

			{

				ColorShader.use();
				glm::mat4 model;
				model = glm::translate(model, LightPos);
				ColorShader.SetMatrix("transformMVP", PVTrans*model);
				ColorShader.SetVec3("lightColor", lightColor*2.0f);
				cubeMesh.Draw(ColorShader);
					
			}

			{
				bool horizontal = true, first_iteration = true;
				unsigned int amount = 10;
				BlurShader.use();
				for (unsigned int i = 0; i < amount; i++)
				{
					glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
					BlurShader.setInt("horizontal", horizontal);
					BlurShader.SetTexture(0, "image", first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);
					screenMesh.Draw(BlurShader);
					horizontal = !horizontal;
					if (first_iteration)
						first_iteration = false;
				}
			}

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			{

				BloomShader.use();
				BloomShader.SetTexture(0, "scene", colorBuffers[0]);
				BloomShader.SetTexture(1, "bloomBlur", pingpongColorbuffers[0]);
				BloomShader.setFloat("exposure", 1);
				screenMesh.Draw(BloomShader);
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

	void GenFrameBuffer(GLuint &hdrFBO, GLuint *colorBuffers)
	{
		glGenFramebuffers(1, &hdrFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		
		glGenTextures(2, colorBuffers);
		for (GLuint i = 0; i < 2; i++)
		{
			glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			// attach texture to framebuffer
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0
			);
		}

		unsigned int rboDepth;
		glGenRenderbuffers(1, &rboDepth);
		glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
		// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
		unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		// finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Framebuffer not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void GenPingPongBuffer(unsigned int *pingpongFBO, unsigned int *pingpongColorbuffers)
	{
		glGenFramebuffers(2, pingpongFBO);
		glGenTextures(2, pingpongColorbuffers);
		for (unsigned int i = 0; i < 2; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
			glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
			// also check if framebuffers are complete (no need for depth buffer)
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "Framebuffer not complete!" << std::endl;
		}
	}
}