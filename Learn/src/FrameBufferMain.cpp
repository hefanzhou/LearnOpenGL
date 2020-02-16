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
#include <windows.h>

namespace FrameBufferMain
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow *window);
	void UpdateTime();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void on_mouse_btn_callback(GLFWwindow*, int button, int action, int mods);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void InitFrameBuffer(unsigned int &framebuffer, unsigned int &texColorBuffer, int, int);

	Camera *camera = nullptr;
	float deltaTime = 0.0f; // 当前帧与上一帧的时间差
	float lastFrame = 0.0f; // 上一帧的时间
	float pai = 3.14159f;
	glm::vec3 mouseInput;
	int screenWidth = 800;
	int screenHeight = 800;
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

		/*COUT<<"Vendor graphic card: %s\n"<< glGetString(GL_VENDOR);
		COUT<<"Renderer: %s\n"<< glGetString(GL_RENDERER);
		COUT<<"Version GL: %s\n"<< glGetString(GL_VERSION);
		COUT<<"Version GLSL: %s\n"<< glGetString(GL_SHADING_LANGUAGE_VERSION);*/

		glViewport(0, 0, screenWidth, screenHeight);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetMouseButtonCallback(window, on_mouse_btn_callback);
		glfwSetScrollCallback(window, scroll_callback);

		Shader ColorShader("./shader/FrameBuffer/SimpleColorShader.vs", "./shader/FrameBuffer/SimpleColorShader.fs");
		Shader TextureShader("./shader/FrameBuffer/TextureShader.vs", "./shader/FrameBuffer/TextureShader.fs");
		Shader RippleShader("./shader/FrameBuffer/RippleShader.vs", "./shader/FrameBuffer/RippleShader.fs");
		Shader ScreenShader("./shader/FrameBuffer/ScreenShader.vs", "./shader/FrameBuffer/ScreenShader.fs");

		camera = new Camera(glm::vec3(0, 5, -5), 0, -45, 45.0f, (float)screenWidth / screenHeight);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthFunc(GL_LESS);

		unsigned int frameBufferId0, frameTextureId0;
		InitFrameBuffer(frameBufferId0, frameTextureId0, screenWidth, screenHeight);

		unsigned int frameBufferId1, frameTextureId1;
		InitFrameBuffer(frameBufferId1, frameTextureId1, screenWidth, screenHeight);

		auto cubeMesh = GetCubeMesh();
		auto cube2Mesh = *cubeMesh;
		auto planMesh = GetPlanMesh(5, 5, 100, 100);
		auto screenMesh = GetScreenMesh();
		auto rippleMesh = screenMesh;
		std::cout.flush();

		{
			glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId0);
			//glClearColor(0.5, 0, 0, 1);
			glDisable(GL_BLEND);
			glClear(GL_COLOR_BUFFER_BIT);

			glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId1);
			//glClearColor(0.5, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			glDisable(GL_BLEND);
		}
		while (!glfwWindowShouldClose(window))
		{
			
			UpdateTime();
			processInput(window);
			
			{
				glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId0);
				glDisable(GL_DEPTH_TEST);
				RippleShader.use();
				RippleShader.SetVec2("iResolution", glm::vec2(screenWidth, screenHeight));
				RippleShader.SetVec3("ripplePos", mouseInput);
				mouseInput.z = 0;
				RippleShader.SetTexture(0, "iChannel0", frameTextureId0);
				RippleShader.SetTexture(1, "iChannel1", frameTextureId1);
				rippleMesh.Draw(RippleShader);

				RippleShader.SetVec3("ripplePos", glm::vec3(0, 0, 0));
				glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId1);
				glDisable(GL_DEPTH_TEST);
				RippleShader.SetTexture(0, "iChannel0", frameTextureId1);
				RippleShader.SetTexture(1, "iChannel1", frameTextureId0);
				rippleMesh.Draw(RippleShader);


			}


			{
				glBindFramebuffer(GL_FRAMEBUFFER, 0); // 返回默认
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glEnable(GL_DEPTH_TEST);
				glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				ScreenShader.use();
				ScreenShader.SetTexture(0, "texture_diffuse", frameTextureId1);
				ScreenShader.SetMatrix("transformMVP", camera->GetPVMatrix());
				planMesh.Draw(ScreenShader);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			}

			glfwSwapBuffers(window);
			glfwPollEvents();

			if (CheckError()) break;
			Sleep(500);
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
		
		COUT << glfwGetMouseButton(window, 0) << endl;
		std::cout.flush();
	}

	void on_mouse_btn_callback(GLFWwindow*, int button, int action, int mods)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			mouseInput.z = 1;
			mouseInput.x = screenWidth/2;
			mouseInput.y = screenHeight/2;

		}
	}


	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		camera->ProcessMouseScroll((float)yoffset);
	}

	void InitFrameBuffer(unsigned int &framebuffer, unsigned int &texColorBuffer, int width, int height)
	{
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		// 生成纹理
		glGenTextures(1, &texColorBuffer);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// 将它附加到当前绑定的帧缓冲对象
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);


		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}