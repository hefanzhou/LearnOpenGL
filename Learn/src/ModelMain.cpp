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

namespace ModelMain
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow *window);
	void UpdateTime();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	Camera *camera = nullptr;
	float deltaTime = 0.0f; // 当前帧与上一帧的时间差
	float lastFrame = 0.0f; // 上一帧的时间
	float pai = 3.14159f / 2.0f;
	
	int main()
	{
		// 初始化
		Log::Init();

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		int screenWidth = 800;
		int screenHeight = 800;
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

		Shader CubeShader("./shader/Model/shader.vs", "./shader/Model/shader.fs");
		Texture texureSpecular("./res/container2_specular.png", true);
		Texture texureDiffuse("./res/container2.png", true);
		Shader LightShader("./shader/Model/LightShader.vs", "./shader/Model/LightShader.fs");
		Shader EadgeShader("./shader/Model/EadgeShader.vs", "./shader/Model/EadgeShader.fs");
		glm::vec3 lightCenter(5.0f, 0.0f, 5.0f);
		glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
		camera = new Camera(glm::vec3(0, 0, 20), 180, 0, 45.0f, (float)screenWidth / screenHeight);
		
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
		
		
		auto cubeMesh = GetCubeMesh();
		std::cout.flush();
		
		Model model("./res/model/nanosuit/nanosuit.obj");
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClearStencil(0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // don't forget to clear the stencil buffer!
			glm::mat4 PVTrans = camera->GetPVMatrix();
			glm::vec3 lightPos;
			{
				LightShader.use();
				glStencilMask(0x00);
				glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

				glm::mat4 modelTrans;
				modelTrans = glm::translate(modelTrans, lightCenter);
				//modelTrans = glm::scale(modelTrans, glm::vec3(0.2f, 0.2f, 0.2f));
				//modelTrans = glm::rotate(modelTrans, pai, glm::vec3(0, 1, 0));
				//modelTrans = glm::translate(modelTrans, glm::vec3(0, 0, 5));
				lightPos = modelTrans * glm::vec4(0, 0, 0, 1);
				auto transformMVP = PVTrans * modelTrans;
				LightShader.SetMatrix("transformMVP", transformMVP);
				LightShader.SetVec3("lightColor", lightColor);
				cubeMesh->Draw(LightShader);
			}

			{
				glClear(GL_STENCIL_BUFFER_BIT);

				CubeShader.use();
				glStencilMask(0xFF);
				glStencilFunc(GL_ALWAYS, 1, 0xFF);

				glm::mat4 modelTrans;
				modelTrans = glm::translate(modelTrans, glm::vec3(0.0f, 0.0f, 0.0f));
				modelTrans = glm::rotate(modelTrans, lastFrame, glm::vec3(0, 1, 0));
				auto transformMVP = PVTrans * modelTrans;
				CubeShader.SetMatrix("transformMVP", transformMVP);
				CubeShader.SetMatrix("transformM", modelTrans);
				CubeShader.SetVec3("lightPos", lightPos);
				CubeShader.SetVec3("lightColor", lightColor);
				CubeShader.SetVec3("viewPos", camera->Position);
				model.Draw(CubeShader);

				//外轮廓
				{
					EadgeShader.use();

					glStencilMask(0x00);
					glDisable(GL_DEPTH_TEST);
					glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

					EadgeShader.SetMatrix("transformMVP", transformMVP);
					EadgeShader.SetMatrix("transformM", modelTrans);
					model.Draw(EadgeShader);

					glEnable(GL_DEPTH_TEST);
					glStencilMask(0xFF);
				}
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