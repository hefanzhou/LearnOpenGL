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

namespace MainGeometryShader
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

		Shader ColorShader("./shader/Geometry/SimpleColorShader.vs", "./shader/Geometry/SimpleColorShader.fs");
		Shader cubeMapShader("./shader/Geometry/CubeMap.vs", "./shader/Geometry/CubeMap.fs");
		Shader TextureShader("./shader/Geometry/Texture.vs", 
			"./shader/Geometry/Texture.fs",
			"./shader/Geometry/Explode.gs");

		Shader visilizationNormalShader("./shader/Geometry/VisualizationNormal.vs",
			"./shader/Geometry/VisualizationNormal.fs",
			"./shader/Geometry/VisualizationNormal.gs");

		camera = new Camera(glm::vec3(0, 0, -5), 0, 0, 45.0f, (float)screenWidth / screenHeight);

		vector<string> faces =
		{
			"./res/skybox/right.jpg",
			"./res/skybox/left.jpg",
			"./res/skybox/top.jpg",
			"./res/skybox/bottom.jpg",
			"./res/skybox/front.jpg",
			"./res/skybox/back.jpg"
		};
		unsigned int cubeTextureId = 0;
		LoadCubeTexture(cubeTextureId, faces);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		auto cubeMesh = GetCubeMesh();
		auto planMesh = GetPlanMesh(5, 5, 10, 10);
		//Model model("./res/model/nanosuit/nanosuit.obj");
		std::cout.flush();
		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // don't forget to clear the stencil buffer!
			glm::mat4 PVTrans = camera->GetPVMatrix();

			{
				glm::mat4 modelTrans;
				modelTrans = glm::translate(modelTrans, glm::vec3(0.0f, -7.0f, 10.0f));
				auto mvpMatrix = PVTrans * modelTrans;
				TextureShader.use();
				TextureShader.setFloat("time", lastFrame);
				TextureShader.SetMatrix("transformMVP", mvpMatrix);
				planMesh.Draw(TextureShader);

				visilizationNormalShader.use();
				visilizationNormalShader.SetMatrix("transformMVP", mvpMatrix);
				visilizationNormalShader.SetMatrix("modelMatrix", modelTrans);
				visilizationNormalShader.SetMatrix("projectionMatrix", camera->PerspectiveMatrix);
				planMesh.Draw(visilizationNormalShader);
			}
			

			{
				cubeMapShader.use();
				glm::mat4 CameraViewNoTranslate = glm::mat4(glm::mat3(camera->ViewMatrix));
				cubeMapShader.SetMatrix("transformVP", camera->PerspectiveMatrix*CameraViewNoTranslate);
				cubeMapShader.SetTexture(0, "texture_diffuse", cubeTextureId, GL_TEXTURE_CUBE_MAP);
				cubeMesh->Draw(cubeMapShader);
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