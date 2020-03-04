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

namespace MainShadow
{
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void processInput(GLFWwindow *window);
	void UpdateTime();
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void InitShadowFrameBuffer(unsigned int &depthbuffer, unsigned int &texDepthBuffer, int width, int height);

	Camera *camera = nullptr;
	float deltaTime = 0.0f; // ��ǰ֡����һ֡��ʱ���
	float lastFrame = 0.0f; // ��һ֡��ʱ��
	float pai = 3.14159f / 2.0f;
	int main()
	{
		// ��ʼ��
		Log::Init();

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		int screenWidth = 800;
		int screenHeight = 800;

		int shadowWidth = 1024;
		int shasowHeight = 1024;
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


		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		Shader TextureShader("./shader/Shadow/TextureShader.vs", "./shader/Shadow/TextureShader.fs");
		Shader ColorShader("./shader/Shadow/ColorShader.vs", "./shader/Shadow/ColorShader.fs");
		Shader GenShadowShader("./shader/Shadow/GenShadow.vs", "./shader/Shadow/GenShadow.fs");
		

		camera = new Camera(glm::vec3(0, 5, -10), 0, -25, 45.0f, (float)screenWidth / screenHeight);
		

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		unsigned int depthBuffer = 0;
		unsigned int depthTex = 0;
		InitShadowFrameBuffer(depthBuffer, depthTex, shadowWidth, shasowHeight);
		auto planMesh = GetPlanMesh(50, 50, 100, 100);
		auto cubeMesh = GetCubeMesh();
		Texture cubeTexture("./res/container.jpg", false);

		std::cout.flush();
		while (!glfwWindowShouldClose(window))
		{
			UpdateTime();
			processInput(window);
			glm::vec3 LightPos(3, 3, 3);
			glm::vec3 LightDir = glm::vec3(0, 0, 0) - LightPos;
			glm::normalize(LightDir);

			GLfloat near_plane = 1.0f, far_plane = 7.5f;
			glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
			glm::mat4 lightView = glm::lookAt(LightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			// ��Ӱͼ
			{
				glViewport(0, 0, shadowWidth, shasowHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, depthBuffer);
				glClear(GL_DEPTH_BUFFER_BIT);
				glCullFace(GL_FRONT);

				GenShadowShader.use();
				{
					glm::mat4 model;
					model = glm::translate(model, glm::vec3(0, 0.5, 0));
					GenShadowShader.SetMatrix("transformMVP", lightProjection*lightView*model);
					cubeMesh->Draw(GenShadowShader);
				}

				{
					glm::mat4 model;
					model = glm::translate(model, glm::vec3(0, 0, 0));
					GenShadowShader.SetMatrix("transformMVP", lightProjection*lightView*model);
					planMesh.Draw(GenShadowShader);
				}

				glCullFace(GL_BACK);
			}

			// ��ʵ����
			{
				glm::mat4 PVTrans = camera->GetPVMatrix();
				glViewport(0, 0, screenWidth, screenHeight);
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				
				{
					TextureShader.use();
					glm::mat4 model;
					model = glm::translate(model, glm::vec3(0, 0.5, 0));
					TextureShader.SetMatrix("transformMVP", PVTrans*model);
					TextureShader.SetMatrix("lightSpaceMatrix", lightProjection*lightView*model);
					TextureShader.SetTexture(0, "texture_diffuse", cubeTexture.GetTextureID());
					TextureShader.SetTexture(1, "textureShadow", depthTex);
					cubeMesh->Draw(TextureShader);
				}

				{
					ColorShader.use();
					glm::mat4 model;
					model = glm::translate(model, glm::vec3(0, 0, 0));
					ColorShader.SetMatrix("transformMVP", PVTrans*model);
					TextureShader.SetMatrix("lightSpaceMatrix", lightProjection*lightView*model);
					ColorShader.SetTexture(0, "textureShadow", depthTex);
					planMesh.Draw(ColorShader);
				}

			}
			
		
			glfwSwapBuffers(window);
			glfwPollEvents();

			if (CheckError()) break;
		}

		glfwTerminate();
		return 0;
	}


	void InitShadowFrameBuffer(unsigned int &depthbuffer, unsigned int &texDepthBuffer, int width, int height)
	{
		glGenFramebuffers(1, &depthbuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, depthbuffer);

		// ��������
		glGenTextures(1, &texDepthBuffer);
		glBindTexture(GL_TEXTURE_2D, texDepthBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glBindTexture(GL_TEXTURE_2D, 0);

		// �������ӵ���ǰ�󶨵�֡�������
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texDepthBuffer, 0);
		// ֻ����Ȼ��壬����д��ɫ����
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}
	
	glm::mat4 * InitTransformArray(unsigned int amount)
	{
		glm::mat4 *modelMatrices;
		modelMatrices = new glm::mat4[amount];
		srand(glfwGetTime()); // ��ʼ���������    
		float radius = 50.0;
		float offset = 10.5f;
		for (unsigned int i = 0; i < amount; i++)
		{
			glm::mat4 model;
			// 1. λ�ƣ��ֲ��ڰ뾶Ϊ 'radius' ��Բ���ϣ�ƫ�Ƶķ�Χ�� [-offset, offset]
			float angle = (float)i / (float)amount * 360.0f;
			float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float x = sin(angle) * radius + displacement;
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float y = displacement * 0.4f; // �����Ǵ��ĸ߶ȱ�x��z�Ŀ��ҪС
			displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
			float z = cos(angle) * radius + displacement;
			model = glm::translate(model, glm::vec3(x, y, z));

			// 2. ���ţ��� 0.05 �� 0.25f ֮������
			float scale = (rand() % 20) / 100.0f + 0.05;
			model = glm::scale(model, glm::vec3(scale));

			// 3. ��ת������һ�����룩���ѡ�����ת�����������������ת
			float rotAngle = (rand() % 360);
			model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

			// 4. ��ӵ������������
			modelMatrices[i] = model;
		}

		return modelMatrices;
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