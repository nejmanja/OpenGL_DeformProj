//TODO: IZKOMENTARISI JEBENI KOD DEBILU

#include<GLAD\glad.h>
#include<GLFW\glfw3.h>
#include<glm\glm.hpp>
#include<glm\gtc\matrix_transform.hpp>
#include<glm\gtc\type_ptr.hpp>
#include<ft2build.h>
#include FT_FREETYPE_H

#include<iostream>
#include<string>
#include "shader.h"
#include "camera.h"
#include "model.h"
#include "textRendering.h"

#define __CAMSPEED 0.005f

//Function prototypes
void loadTexture(unsigned int* texture, bool isRGBA, const char* path);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
unsigned int loadCubemap(vector<std::string> faces);
bool basicRayCheck(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 rayOrigin, glm::vec3 rayDir);
void renderRay(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::mat4 view, glm::mat4 model, glm::mat4 projection, Shader& shader);

//Global variables
int windowWidth = 800, windowHeight = 600;
float deltaTime = 0.0f, lastFrame = 0.0f;
float mousePitch = 0.0f, mouseYaw = 0.0f;
float fov = 45.0f;
int shineStr = 32;
float linear = 0.045f, quadratic = 0.0075;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = windowWidth / 2.0f;
float lastY = windowHeight / 2.0f;
bool firstMouse = true;

int main()
{
	//------------------------------------------------------------------------------------------------
	//Initialization
	//------------------------------------------------------------------------------------------------

	//Window init
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "bottom text", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create window!" << std::endl;
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Loading opengl func pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD!" << std::endl;
		return -1;
	}

	Text text("../OpenGL_DeformProj/arial.ttf", 0, 48, glm::vec3(0.5f, 0.8f, 0.2f));

	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE); //enable culling
	glCullFace(GL_BACK); //tell opengl to cull back faces
	glFrontFace(GL_CCW); //set the front faces to be counter-clockwise winded

	//------------------------------------------------------------------------------------------------
	//Geometry and shader setup
	//------------------------------------------------------------------------------------------------

	Shader objShader("../OpenGL_DeformProj/objVert.vert", "../OpenGL_DeformProj/objFrag_noTex.frag");
	Shader lightShader("../OpenGL_DeformProj/lightVert.vert", "../OpenGL_DeformProj/lightFrag.frag");
	Shader singleColorShader("../OpenGL_DeformProj/singleColor.vert", "../OpenGL_DeformProj/singleColor.frag");
	Shader textShader("../OpenGL_DeformProj/textShader.vert", "../OpenGL_DeformProj/textShader.frag");
	Shader skyboxShader("../OpenGL_DeformProj/skybox.vert", "../OpenGL_DeformProj/skybox.frag");

	Shader rayShader("../OpenGL_DeformProj/ray.vert", "../OpenGL_DeformProj/ray.frag");

	float vertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		// Front face
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		// Left face
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		// Right face
		 0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 // Bottom face
		 -0.5f, -0.5f, -0.5f,
		  0.5f, -0.5f, -0.5f,
		  0.5f, -0.5f,  0.5f,
		  0.5f, -0.5f,  0.5f,
		 -0.5f, -0.5f,  0.5f,
		 -0.5f, -0.5f, -0.5f,
		 // Top face
		 -0.5f,  0.5f, -0.5f,
		  0.5f,  0.5f,  0.5f,
		  0.5f,  0.5f, -0.5f,
		  0.5f,  0.5f,  0.5f,
		 -0.5f,  0.5f, -0.5f,
		 -0.5f,  0.5f,  0.5f
	};
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};
	glm::vec3 lightPositions[] = {
		glm::vec3(3.0f,  1.0f,  10.0f),
		glm::vec3(11.0f, -15.0f, -20.0f),
		glm::vec3(-20.0f,  10.0f, -60.0f),
		glm::vec3(0.0f,  0.0f, -15.0f)
	};

	//Light (cube) geometry setup
	unsigned int VBO, EBO, lampVAO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &lampVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(lampVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	//Skybox setup
	std::vector<std::string> skyBoxFaces = {
		"../../OpenGLAssets/Skybox/right.jpg",
		"../../OpenGLAssets/Skybox/left.jpg",
		"../../OpenGLAssets/Skybox/top.jpg",
		"../../OpenGLAssets/Skybox/bottom.jpg",
		"../../OpenGLAssets/Skybox/front.jpg",
		"../../OpenGLAssets/Skybox/back.jpg"
	};
	unsigned int cubemapTexture = loadCubemap(skyBoxFaces);
	unsigned int skyVAO, skyVBO;
	glGenVertexArrays(1, &skyVAO);
	glGenBuffers(1, &skyVBO);
	glBindVertexArray(skyVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	skyboxShader.use();
	skyboxShader.setInt("skyBox", 0);

	//Static light setup
	objShader.use();
	glm::vec3 lightDiffuse = glm::vec3(0.66f, 0.86f, 0.97f);
	for (int i = 0; i < 4; i++)
	{
		objShader.setPointLightAt("pointLights", i, lightPositions[i], lightDiffuse, linear, quadratic);
	}
	glm::vec3 sunDiffuse = glm::vec3(1.0f, 0.7f, 0.3f);
	glm::vec3 sunDirection = glm::vec3(-0.2f, -1.0f, -0.3f);
	objShader.setDirectionalLight("dirLight", sunDirection, sunDiffuse);

	Model target("../../OpenGLAssets/testBottle/testBottle.obj");
	objShader.setVec3("material.diffuse", target.material.diffuse);
	objShader.setVec3("material.specular", target.material.specular);

	//------------------------------------------------------------------------------------------------
	//Main loop
	//------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		//Clearing leftover data
		glClearColor(.1f, .1f, .1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//Setting up the matrices
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 projection;
		view = camera.GetViewMatrix();
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

		objShader.use();
		objShader.setMat4("view", view);
		objShader.setMat4("projection", projection);
		objShader.setVec3("viewPos", camera.Position);
		objShader.setFloat("material.shininess", 32.0f); //the only thing left in the material not managed by textures
		singleColorShader.setVec3("viewPos", camera.Position);
		singleColorShader.setFloat("material.shininess", 32.0f);

		//Dynamic light setup
		objShader.setSpotLight("flashLight", camera.Position, camera.Front, lightDiffuse, 12.5f, 17.5f);

		glDepthMask(GL_FALSE);
		// draw skybox as last
		skyboxShader.use();
		// remove translation from the view matrix
		skyboxShader.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
		skyboxShader.setMat4("projection", projection);
		// skybox cube
		glBindVertexArray(skyVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthMask(GL_TRUE);

		//Rendering light geometry
		glBindVertexArray(lampVAO);
		for (int i = 0; i < 4; i++)
		{
			lightShader.use();
			model = glm::mat4(1.0f);
			model = glm::scale(model, glm::vec3(.5f, .5f, .5f));
			model = glm::translate(model, lightPositions[i]);
			lightShader.setMat4("view", view);
			lightShader.setMat4("model", model);
			lightShader.setMat4("projection", projection);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		//Rendering the target
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		model = glm::rotate(model, (float)glm::radians(glfwGetTime() * 40), glm::vec3(1.0f, 0.3f, 0.5f));
		objShader.use();
		objShader.setMat4("model", model);
		target.Draw(objShader);

		model = glm::mat4(1.0f);
		renderRay(glm::vec3(-1.0f, -1.0f, 0.0f), glm::normalize(glm::vec3(2.0f, 1.0f, 3.0f)), view, model, projection, rayShader);
		bool rayResult = basicRayCheck(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(-1.0f, -1.0f, 0.0f), glm::normalize(glm::vec3(2.0f, 1.0f, 3.0f)));

		//Rendering text
		glm::mat4 textCanvas = glm::ortho(0.0f, (float)windowWidth, 0.0f, (float)windowHeight);
		text.renderText(textShader, "FPS:" + std::to_string((int)(1 / deltaTime)), 0.0f, windowHeight - 48.0f, 1.0f, textCanvas);
		//if (rayResult == glm::vec3(0, 0, 0))
		//{
		//	text.renderText(textShader, "no hit haha yes", 0.0f, windowHeight - 96.0f, 1.0f, textCanvas);
		//}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//glDeleteFramebuffers(1, &fbo);
	glfwTerminate();
	return 0;
}

//Handles all the input
void processInput(GLFWwindow * window)
{
	//Quitting the window on escape press
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}
	else if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	//Camera movement
	float camSpeed = 2.5f * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
	{
		linear -= deltaTime;
		if (linear <= 0.0014)
			linear = 0.0014;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
	{
		linear += deltaTime;
		if (linear >= 0.7)
			linear = 0.7;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
	{
		quadratic -= deltaTime / 10.0f;
		if (quadratic <= 0.000007)
			quadratic = 0.000007;
	}
	else if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
	{
		quadratic += deltaTime / 10.0f;
		if (quadratic >= 1.8)
			quadratic = 1.8;
	}

}

void renderRay(glm::vec3 rayOrigin, glm::vec3 rayDir, glm::mat4 view, glm::mat4 model, glm::mat4 projection, Shader& shader)
{
	//Render the ray (debug purposes)
	glm::vec3 vertices[] = { rayOrigin, rayOrigin + rayDir * 10000.0f };
	unsigned int VAO, VBO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); //position vertex attribute
	glEnableVertexAttribArray(0);

	shader.use();
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool basicRayCheck(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 rayOrigin, glm::vec3 rayDir)
{
	//Do actual math lmao
	glm::vec3 normal = glm::cross((v1 - v0), (v2 - v0));

	float planeDistance = glm::dot(normal, v0);
	float parallelChecker = glm::dot(normal, rayDir);
	if (parallelChecker == 0)
		return false;
	float hitDistance = -(glm::dot(normal, rayOrigin) + planeDistance) / parallelChecker;
	if (hitDistance < 0)
		return false;
	glm::vec3 rayHit = rayOrigin + hitDistance * rayDir;

	glm::vec3 edge0 = v1 - v0;
	glm::vec3 edge1 = v2 - v1;
	glm::vec3 edge2 = v0 - v2;
	glm::vec3 C0 = rayHit - v0;
	glm::vec3 C1 = rayHit - v1;
	glm::vec3 C2 = rayHit - v2;
	if (glm::dot(edge0, C0) < 0)
		return false;
	if (glm::dot(edge1, C1) < 0)
		return false;
	if (glm::dot(edge2, C2) < 0)
		return false;

	return true;
}

void mouse_callback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

//Loading textures from external source with assumed wrapping parameters
void loadTexture(unsigned int* texture, bool isRGBA, const char* path)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height, nrChans;
	unsigned char* data = stbi_load(path, &width, &height, &nrChans, 0);
	if (data)
	{
		if (isRGBA)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "ERROR: Failed to load image data!";
	}
	stbi_image_free(data);
}

//Callback for resizing the window
void framebufferSizeCallback(GLFWwindow * window, int width, int height)
{
	glViewport(0, 0, width, height);
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
