#define _CRT_SECURE_NO_WARNINGS

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "shader_m.h"
#include "camera.h"

#include <vector>
#include <string>
#include <iostream>

//#include <Eigen/Dense>

#include "loadobj.h"

#include "tangentspace.h"
#include "vboindexer.h"
#include "texture.h"


GLfloat rotate = 0.0f;
GLfloat rotationStep = 0.001f;
bool animate = false;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
unsigned int loadCubemap(std::vector<std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1706;// 1280;//2560;//1280;
const unsigned int SCR_HEIGHT = 1280;//1920;//720;

// camera
//Camera camera(glm::vec3(0.0f, -1.0f, 0.0f)); //1 person
Camera camera(glm::vec3(0.0, -1.0f,3.0f));  //overview
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool useNormal = true;

float yawTeapot = 0.0f;
float pitchTeapot = 0.0f;
float rollTeapot = 0.0f;
float rotSpeed = 0.01f;

bool useEuler = true;
bool useQuaternion = false;

bool pressYaw = false;
bool pressPitch = false;
bool pressRoll = false;

bool personCam = false;

glm::mat4 inverseModel;
int main()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Rotating Plane", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	// shaders 
	//Shader shader("../Shaders/Vert.vs", "../Shaders/Frag.fs");
	Shader shader("../Shaders/SimpleTexture.vs", "../Shaders/SimpleTexture.fs");
	//Shader shaderNorm("../Shaders/NormalMap.vs", "../Shaders/NormalMap.fs");
	Shader shaderNorm("../Shaders/SimpleTexture.vs", "../Shaders/SimpleTexture.fs");
	Shader shaderWithoutNorm("../Shaders/SimpleTexture.vs", "../Shaders/SimpleTexture.fs");
	Shader skyboxShader("../Shaders/skybox.vs", "../Shaders/skybox.fs");
	
	float skyboxVertices[] = {         
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

	// teapot VAO
	unsigned int teapotVAO;
	glGenVertexArrays(1, &teapotVAO);
	glBindVertexArray(teapotVAO);

	//load object
	std::vector<glm::vec3> teapotVertices;
	std::vector<glm::vec2> teapotTextures;
	std::vector<glm::vec3> teapotNormals;
	bool res = loadOBJ("../Models/plgabo.obj", teapotVertices, teapotTextures, teapotNormals);
	//bool res = loadOBJ("../Models/plwithoutpropeii.obj", teapotVertices, teapotTextures, teapotNormals);

	// Load it into a VBO
	GLuint tvertexbuffer;
	glGenBuffers(1, &tvertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tvertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, teapotVertices.size() * sizeof(glm::vec3), &teapotVertices[0], GL_STATIC_DRAW);

	GLuint tuvbuffer;
	glGenBuffers(1, &tuvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tuvbuffer);
	glBufferData(GL_ARRAY_BUFFER, teapotTextures.size() * sizeof(glm::vec2), &teapotTextures[0], GL_STATIC_DRAW);

	GLuint tnormbuffer;
	glGenBuffers(1, &tnormbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tnormbuffer);
	glBufferData(GL_ARRAY_BUFFER, teapotNormals.size() * sizeof(glm::vec3), &teapotNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, tvertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, tuvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, tnormbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	


	// propeller VAO
	unsigned int propellerVAO;
	glGenVertexArrays(1, &propellerVAO);
	glBindVertexArray(propellerVAO);
	//load propeller
	std::vector<glm::vec3> propellerVertices;
	std::vector<glm::vec2> propellerTextures;
	std::vector<glm::vec3> propellerNormals;
	res = loadOBJ("../Models/propeller_Final.obj", propellerVertices, propellerTextures, propellerNormals);
	//res = loadOBJ("../Models/Free_Propeller_1.obj", propellerVertices, propellerTextures, propellerNormals);

	// Load it into a VBO
	GLuint pvertexbuffer;
	glGenBuffers(1, &pvertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pvertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, propellerVertices.size() * sizeof(glm::vec3), &propellerVertices[0], GL_STATIC_DRAW);
	
	GLuint puvbuffer;
	glGenBuffers(1, &puvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, puvbuffer);
	glBufferData(GL_ARRAY_BUFFER, propellerTextures.size() * sizeof(glm::vec2), &propellerTextures[0], GL_STATIC_DRAW);

	GLuint pnormbuffer;
	glGenBuffers(1, &pnormbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, pnormbuffer);
	glBufferData(GL_ARRAY_BUFFER, propellerNormals.size() * sizeof(glm::vec3), &propellerNormals[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, pvertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, puvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, pnormbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	


	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// textures
	GLuint DifCubeTex = loadDDS("../Texture/brick.DDS");
	GLuint NormCubeTex = loadBMP_custom("../Texture/brick_norm.bmp");
	//GLuint NormCubeTex = loadBMP_custom("../Texture/brick_norm_test.bmp");

	GLuint DifpropellerTex = loadDDS("../Texture/mud_dif.DDS");
	GLuint NormpropellerTex = loadBMP_custom("../Texture/mud_norm.bmp");
	//GLuint NormpropellerTex = loadBMP_custom("../Texture/brick_norm_test.bmp");
	GLuint SpecpropellerTex = loadDDS("../Texture/mud_spec.DDS");

	GLuint DiffuseTex = loadDDS("../Texture/cbp.DDS");
	//GLuint NormalTex = loadBMP_custom("../Texture/bark_norm.bmp");
	//GLuint SpecularTex = loadDDS("../Texture/cbp.DDS");


	std::vector<std::string> faces
	{
		"../Texture/right.jpg",
		"../Texture/left.jpg",
		"../Texture/top.jpg",
		"../Texture/bottom.jpg",
		"../Texture/front.jpg",
		"../Texture/back.jpg"
	};

	unsigned int cubemapTexture = loadCubemap(faces);


	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);


	while (!glfwWindowShouldClose(window))
	{

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//camera.setDisableMouse(true);
		processInput(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//DRAW
		glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		//draw teapot
		glm::mat4 modelPlane = glm::mat4(1.0f);
		modelPlane = glm::scale(modelPlane, glm::vec3(0.2f, 0.2f, 0.2f));
		modelPlane = glm::translate(modelPlane, glm::vec3(0.0f, -8.0f, 0.0f));
		modelPlane = glm::rotate(modelPlane, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 R;
		glm::quat orientation;

		/// Process input and modify plane yaw, pitch and roll
		if (useEuler) {
			//Euler Angles
			R = glm::yawPitchRoll(yawTeapot, pitchTeapot, rollTeapot); //rotation matrix Euler Angles
			//camera.setDirection(glm::mat3(R) * glm::vec3(0, 0, -1));
			modelPlane = modelPlane * R;
		}
		if (useQuaternion) {
			//quaternions-------------------------------------------------------------
			glm::quat qPitch = glm::angleAxis(pitchTeapot, glm::vec3(1, 0, 0));
			glm::quat qYaw = glm::angleAxis(yawTeapot, glm::vec3(0, 1, 0));
			glm::quat qRoll = glm::angleAxis(rollTeapot, glm::vec3(0, 0, 1));



			//----------------------------------------------------------	PITCH
			if (pressYaw) {
				orientation = qPitch * qYaw;
			}
			//-------------------------------------------------------------	YAW
			if (pressRoll) {
				orientation = qPitch * qRoll;
			}
			//------------------------------------------------------------- ROLL 
			if (pressPitch) {
				orientation = qYaw * qRoll;
			}

			//endquat--------------------------------------------------------------------

			orientation = glm::normalize(orientation);
			glm::mat4 rotate = glm::mat4_cast(orientation);


			glm::mat4 translate = glm::mat4(1.0f);
			//translate = glm::translate(translate, -(glm::vec3(4.0f)*camera.Position));
			translate = glm::translate(translate, glm::vec3(0.0f, 0.0f, -15.0f));
			translate = glm::translate(translate, glm::vec3(0.0f, -6.0f, 0.0f));

			glm::mat4 rotate2 = glm::mat4(1.0f);
			rotate2 = glm::rotate(rotate2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			//rotate2 = glm::rotate(rotate2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 1.0f));


			modelPlane = translate * rotate2 * rotate;
			//modelPlane = translate * rotate;
			//modelTeapot = rotate;

		}


		//camera.setPosition(glm::vec3(0.0f, -1.0f, 0.2f));

		glm::mat4 viewTeapot = camera.GetViewMatrix();
		glm::mat4 mvpTeapot = projection_matrix * viewTeapot * modelPlane;
		glm::vec3 lightPos = glm::vec3(0, 1, -5);
		//glm::vec3 lightPos = glm::vec3(0, 3, -7);

		shader.use();
		shader.setMat4("mvp", mvpTeapot);
		shader.setMat4("view", viewTeapot);
		//shader.setMat4("view", glm::inverse(modelTeapot));
		shader.setMat4("model", modelPlane);
		shader.setVec3("LightPosW", lightPos);

		glBindVertexArray(teapotVAO);
		//textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, DiffuseTex);
		glDrawArrays(GL_TRIANGLES, 0, teapotVertices.size());



		//draw propeller
		glm::mat4 modelpropeller = glm::mat4(1.0f);
		//viewpropeller = glm::rotate(viewpropeller, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); //for 1 person

		glm::mat4 projpropeller = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		
		
		modelpropeller = modelPlane;
		//modelpropeller = glm::rotate(modelpropeller, glm::radians(rotate)* 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
		
		modelpropeller = glm::scale(modelpropeller, glm::vec3(1.5f, 1.5f, 1.5f));
		modelpropeller = glm::rotate(modelpropeller, glm::radians(75.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		modelpropeller = glm::translate(modelpropeller, glm::vec3(2.1f, -0.17f, -0.5f));

		//modelpropeller = glm::translate(modelpropeller, glm::vec3(-0.0f, 2.7f, 0.7f)); ///3.17f

		//modelpropeller = glm::translate(modelpropeller, glm::vec3(2.1f, 0.2f, 0.0f));
		//modelpropeller = glm::rotate(modelpropeller, glm::radians(rotate) * 90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	
		glm::mat4 mvppropeller = projpropeller * viewTeapot * modelpropeller;

		//glm::vec3 lightPos = glm::vec3(0, 1, 3);
		glm::vec3 lightPosP = glm::vec3(-2, 4, -4);

		shader.use();
		shader.setMat4("mvp", mvppropeller);
		shader.setMat4("view", viewTeapot);
		shader.setMat4("model", modelpropeller);
		shader.setVec3("LightPosW", lightPosP);

		glBindVertexArray(propellerVAO);
		//textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, DifCubeTex);
		glDrawArrays(GL_TRIANGLES, 0, propellerVertices.size());

		// skybox
		glDepthFunc(GL_LEQUAL);  
		skyboxShader.use();
		glm::mat4 model = glm::mat4(1.0f);
	    //model = glm::inverse(modelTeapot);
		//model = modelTeapot;


		skyboxShader.setMat4("modelsky", model);
		//skyboxShader.setMat4("viewsky", view);
		//inverseModel = glm::inverse(model) * glm::mat4(glm::mat3(camera.GetViewMatrix()));
		//view = glm::mat4(glm::mat3(camera.GetViewMatrix()))* modelTeapot ;
		//view = glm::translate(viewTeapot, glm::vec3(0., 0., 0.5));
		
		//camera.setFirstPerson(yawTeapot, pitchTeapot, rollTeapot);
		/*glm::mat3 vie = glm::mat3(camera.GetViewMatrix());
		glm::mat3 rot = glm::mat3(R);
		vie = vie * rot;*/
		//view = glm::mat4(vie);

		
		 //--------------
		if (personCam) {
			//first person
			glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
			glm::vec3 scal;
			glm::quat orient;
			glm::vec3 trans;
			glm::vec3 skew;
			glm::vec4 persp;
			glm::decompose(modelPlane, scal, orient, trans, skew, persp);
			//---------------------

		   camera.setPosition(glm::vec3(0.0f, -1.0f, 0.0f));

			//-------------------------
		   //orient = -orient;
		   glm::mat4 newView =  glm::toMat4(orient);
		   newView = newView * rotate;

			//-------------------
		   /*glm::mat3 modeltemp = glm::mat3(modelTeapot);
		   modeltemp = normalize(modeltemp);
		   glm::mat4 viewtemp = glm:mat4(modeltemp);*/

		   skyboxShader.setMat4("viewsky", newView);
		   

		}
		else {
			camera.setPosition(glm::vec3(0.0, -1.0f, 3.0f));
			glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
			skyboxShader.setMat4("viewsky", view);
		}



		/*camera.setPosition(glm::vec3(0.0f, -1.5f, -3.9f));
		glm::mat4 newView = glm::mat4_cast(orientation);*/

		//camera.updateCameraVectors2(yawTeapot, pitchTeapot, rollTeapot);
		/*glm::mat4 viewtemp = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		viewtemp = viewtemp * R;*/
		

	
		
		// first person
		//
		
		skyboxShader.setMat4("projectionsky", projection_matrix);
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS); 

		rotate += rotationStep;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVAO);

	glfwTerminate();
	return 0;
}

// keyboard controls
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	//UP & DOWN
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(SUS, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(JOS, deltaTime);
	//start/stop animation
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		rollTeapot += rotSpeed; 
		pressYaw = false;
		pressPitch = true;
		pressRoll = false;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		rollTeapot -= rotSpeed;
		pressYaw = false;
		pressPitch = true;
		pressRoll = false;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		yawTeapot += rotSpeed;
		pressYaw = true;
		pressPitch = false;
		pressRoll = false;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
		yawTeapot -= rotSpeed;
		pressYaw = true;
		pressPitch = false;
		pressRoll = false;
	}
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS){
		pitchTeapot += rotSpeed;
		pressYaw = false;
		pressPitch = false;
		pressRoll = true;
	}
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS){
		pitchTeapot -= rotSpeed;
		pressYaw = false;
		pressPitch = false;
		pressRoll = true;
	}
	
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		if (!animate)
			animate = true;
		else
			animate = false;


	//use Euler/ use Quaternion
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
		useEuler = true;
		useQuaternion = false;
	}
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
		useEuler = false;
		useQuaternion = true;
	}
	//camera
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		personCam = true;
	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
		personCam = false;
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
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
