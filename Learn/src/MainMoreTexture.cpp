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

namespace MainMoreTexture
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow *window);
	void UpdateTime();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


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

		Shader NormalTextureShader("./shader/MoreTexture/NormalTexture.vs", "./shader/MoreTexture/NormalTexture.fs");
		Shader ColorShader("./shader/MoreTexture/Light.vs", "./shader/MoreTexture/Light.fs");
		camera = new Camera(glm::vec3(0, 0, -5), 0, 0, 45.0f, (float)screenWidth / screenHeight);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glViewport(0, 0, screenWidth, screenHeight);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		auto planMesh = GetPlanMesh(2, 2, 2, 2);
		Texture diffsuleTexture("./res/bricks2.jpg", false);
		Texture normalTexture("./res/bricks2_normal.jpg", false);
		Texture dispTexture("./res/bricks2_disp.jpg", false);

		std::cout.flush();
		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glm::vec3 LightPos(0, 3, -3);

			
			glm::mat4 PVTrans = camera->GetPVMatrix();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//if(false)
			{
				NormalTextureShader.use();
				glm::mat4 model;
				model = glm::translate(model, glm::vec3(0, 0, 0));
				model = glm::rotate(model, -pai / 2.0f, glm::vec3(1, 0, 0));

				NormalTextureShader.SetMatrix("transformMVP", PVTrans*model);
				NormalTextureShader.SetMatrix("transformM", model);
				NormalTextureShader.SetVec3("lightPos", LightPos);
				NormalTextureShader.SetVec3("viewPos", camera->Position);
				NormalTextureShader.SetTexture(0, "diffsuleTexture", diffsuleTexture.GetTextureID());
				NormalTextureShader.SetTexture(1, "normalTexture", normalTexture.GetTextureID());
				NormalTextureShader.SetTexture(2, "depthMap", dispTexture.GetTextureID());

				planMesh.Draw(NormalTextureShader);
			}
			if (false)
			{
				ColorShader.use();
				glm::mat4 model;
				model = glm::translate(model, glm::vec3(0, 0, 0));
				model = glm::rotate(model, -pai / 2.0f, glm::vec3(1, 0, 0));

				ColorShader.SetMatrix("transformMVP", PVTrans*model);
				planMesh.Draw(ColorShader);
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
}