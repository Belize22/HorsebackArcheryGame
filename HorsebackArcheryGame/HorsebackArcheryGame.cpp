/***************************************************************************************************\
| Program:     Horseback Archery Game                                                               |
| Developer:   Peter Irshad                                                                         |
| Description: Currently, this program provides the following functionality:                        |
|              - Generation of 20 horses with varying orientations and sizes.                       |
|              - Randomized horse movement (they move straight for a random amount of steps from 10 |
|                to 30 before rotating 30 degrees left or right.                                    |
|              - Collision detection between each horse (if they collide, one randomly stops while  |
|                the other goes).                                                                   |
|              - The user can toggle whether the entire scene is still or animated.                 |
|              - Each horse randomly changes speed and also randomly stops for a random amount of   |
|                time                                                                               |
|              - Each horse has animations. Run animation for going fast, Walk animation for going  |
|                slow and jump animation for randomly stopping.                                     |
|              - The user can select a horse to control. When controlling a horse, the user can:    |
|                move (straight, left or right), modify speed and stop to do a jump                 |
|              - Giving the horse a horse skin texture and the grid a grass texture. Can be toggled |
|                on and off.                                                                        |
|              - Adds lighting to the scene based on the Phong Model.                               |
|              - Allows horse to rotate joints by utilizing Hierarchical modeling based on a tree   |
|                structure and a matrix stack.                                                      |
|              - Horse movement and horse rotation                                                  |
|              - Pan and tilt world orientation.                                                    |
|              - Scene can be rendered using triangles (full body), lines (wire mesh), and points   |
|                (index indicator).                                                                 |
|              - Panning, tilting and zooming of camera.                                            |
|              - When window is resized, the proportions remain proper while maintaining the        |
|                central position of the camera.                                                    |
\***************************************************************************************************/

#include "stdafx.h"

#include "..\glew\glew.h"	//include GL Extension Wrangler
#include "..\glfw\glfw3.h"	//include GLFW helper library
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <random>           //For rand() function.
#include <time.h>           //For time() function.
#include <vector>           //For a list based data structure with dynamic sizing.
#include <math.h>           //For sqrt() function.

//Include GLM and all it's supplemental libraries.
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"

//Include the class that represents a horse (this class contains the stack, node and tree data structures).
#include "Horse.h"

//For image loading.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//To ease use of std operations.
using namespace std;

//Numeric constants
const float PI = 3.14f; //Used for rotation transformations (degrees are in radians).
const int MIN = -50;    //Used to indicate minimum coordinate for randomization code (square grid so MIN is same for x-axis and z-axis)
const int MAX = 50;     //Used to indicate maximum coordinate for randomization code (square grid so MAX is same for x-axis and z-axis)

//Initial window dimensions.
const GLuint INITIAL_WIDTH = 800, INITIAL_HEIGHT = 800;

//Current window dimensions.
GLuint WIDTH = INITIAL_WIDTH, HEIGHT = INITIAL_HEIGHT;

//Resolution for shadow map (i.e. the lower it is, the more pixely shadows are)
const GLint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

glm::vec4 WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

int HORSES = 20;        //Amount of horses to generate in the scene.

//Indication of whether various mouse buttons are being held or not.
bool leftMouseHold = false;
bool middleMouseHold = false;
bool rightMouseHold = false;

//These variables keep track of current cursor position and previous cursor position within a small moment of time in order to provide
//necessary movement when camera movement is done (only for mouse based inputs tough).
double currentXPos;
double currentYPos;
double previousXPos;
double previousYPos;
double xPosDifference;                    //The difference is between the current and previous positions.
double yPosDifference;

//These variables affect camera position. Default position of camera is above the horse on the y-axis.
float cameraPan = 0.0f;                   //Indicate how much we rotate around the y-axis.
float cameraTilt = 0.0f;                  //Indicate how much we rotate around the x-axis
float translateX = 0.0f;                  //Indicate how much we pan to the left/right.
float zoomValue = PI / 4;                 //Indicate how much we zoom forward/backward.

//These variables affect the orientation of all the objects in the scene.
float worldPan = 0.0f;                    //Indicate how much the world pans leftward/rightward.
float worldTilt = 0.0f;                   //Indicate how much the world tilts upward/downward.

//These variables affect the scale of the window in relation to it's initial size.
float windowAdjustmentX = 1.0f;
float windowAdjustmentY = 1.0f;

//Properties indicating how to draw the horse (full color with triangles, a mesh with lines or representation of each indice with points).
int drawType = GL_TRIANGLES;
int indiceQuantity = 6;

//Temporary camera position variables used to influence the specular lighting.
float tempViewPosX = 0.0f;
float tempViewPosY = 0.0f;
float tempViewPosZ = 0.0f;

//Two angles to represent spherical coordinates. This allows easy modification of camera position when it comes to panning and tilting the camera
//(these are associated with the temporary camera positions above).
float theta = 3 * PI / 2;
float phi = 0;

bool recordStartingPos = false;            //Used to indicate that we store the window position (x and y values) when a mouse button has been clicked.
bool preventHoldFunctionality = false;     //Prevents user from holding buttons to repeat horse movement and rotation many times.
bool debugCollisions = false;              //Toggle whether to debug collisions (i.e. Horses turn different colors depending on their collision status).

bool texturesActive = true;                //Indicate whether to render the horse and grid with textures or not.
bool reverseJointRotation = false;         //Indicate whether to rotate the current joint counter-clockwise or not.
bool shadowsActive = false;                //Indicate whether to turn on shadows or not.
bool animationActive = false;              //Indicate whether to keep the scene animation active or not.
bool selectingHorse = false;               //Indicate whether the user is currently selecting a horse.
bool controllingHorse = false;             //Indicate whether the user is controlling a horse.

vector<Horse*> horses;                     //All horses that exist in the scene.
int selectedHorse = 1;

GLuint gridVAO, gridVBO, cubeVAO, cubeVBO;
GLuint transformLoc, viewMatrixLoc, projectionLoc, shadowTransformLoc, shadowViewMatrixLoc1, shadowViewMatrixLoc2, shadowProjectionLoc1, shadowProjectionLoc2, shadowsActiveLoc, objectColorLocation;
unsigned int plainTexture, grassTexture, horseSkinTexture;
glm::mat4 worldRotation;
glm::mat4 model_matrix;

//FUNCTION PROTOTYPES
void updateDrawType();
void updateWorldOrientation();

void character_callback(GLFWwindow* window, unsigned int codepoint);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int newWidth, int newHeight);

GLuint importShaders(string vertex_shader_path, string fragment_shader_path);
unsigned int importTexture(char const *file_path);

float distanceBetweenTwoPoints(float x1, float x2, float y1, float y2, float z1, float z2);
bool sphereCollisionDetection(glm::vec3 pos1, glm::vec3 pos2, float radius1, float radius2);
bool collisionDetected(Horse* horse1, Horse* horse2, forecastDirection direction);
bool collisionDetectedWithControlledHorse(Horse* controlledHorse, Horse* independentHorse);
bool isFartherFromCollision(Horse* stoppedHorse, Horse* avoidingHorse, forecastDirection direction);
void collisionResolutionDuring(Horse* horse1, Horse* horse2);
void collisionResolutionEnd(Horse* horse1, Horse* horse2);

void generateGrid(GLuint shaderProgram);
int randomNumber(int min, int max);

//The MAIN function, from here we start the application and run the game loop
int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;
	glfwInit(); //Init GLFW

	//Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);                         //Allow user to resize window.

	//Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Horseback Archery Game", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Set the required callback functions
	glfwSetCharCallback(window, character_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	//Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;

	//Initialize GLEW to setup the OpenGL Function pointers
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	//Define the viewport dimensions
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glViewport(0, 0, width, height);
	glEnable(GL_DEPTH_TEST);                          //Enable z-buffering.

	//Build and compile our shader program

	GLuint shaderProgram = importShaders("vertex.shader", "fragment.shader");
	GLuint shadowShaderProgram = importShaders("shadowVertex.shader", "shadowFragment.shader");

	glUseProgram(shaderProgram);

	objectColorLocation = glGetUniformLocation(shaderProgram, "objectColor");         //Allow the color of an object to be changed.
	int lightColorLocation = glGetUniformLocation(shaderProgram, "lightColor");       //Allow the light color to be changed.
	int lightPositionLocation = glGetUniformLocation(shaderProgram, "lightPosition"); //Allow modification of variable influencing how bright a face of a
	//shape is when it's within/away from the light.
	int viewPositionLocation = glGetUniformLocation(shaderProgram, "viewPosition");   //Allow modification of temporary camera position of variable to
	//influence specular lighting.

	//GRID PROPERTIES
	//Where vertices for the grid are defined (first 3: position, middle 2: texture, last 3: normals).
	GLfloat gridVertices[] = {
		-50.0, 0.0, -50.0, 0.0, 0.0, 0.0, 1.0, 0.0,
		-50.0, 0.0, 50.0, 0.0, 50.0, 0.0, 1.0, 0.0,
		50.0, 0.0, 50.0, 50.0, 50.0, 0.0, 1.0, 0.0,
		-50.0, 0.0, -50.0, 0.0, 0.0, 0.0, 1.0, 0.0,
		50.0, 0.0, -50.0, 50.0, 0.0, 0.0, 1.0, 0.0,
		50.0, 0.0, 50.0, 50.0, 50.0, 0.0, 1.0, 0.0,
		-50.0, 0.0, -50.0, 0.0, 0.0, 0.0, 1.0, 0.0,
		50.0, 0.0, -50.0, 50.0, 0.0, 0.0, 1.0, 0.0,
		-50.0, 0.0, 50.0, 0.0, 50.0, 0.0, 1.0, 0.0,
		50.0, 0.0, 50.0, 50.0, 50.0, 0.0, 1.0, 0.0,
	};

	//VAO and VBO for the grid.
	glGenVertexArrays(1, &gridVAO);
	glGenBuffers(1, &gridVBO);

	//Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(gridVAO);
	glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0); //Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
	glBindVertexArray(0);             //Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	//CUBE (FOR HORSE BODY PARTS) PROPERTIES.
	//Where vertices for the cube representing horse body parts are defined (first 3: position, middle 2: texture, last 3: normals).
	GLfloat cubeVertices[] = {
		-1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0, -1.0,
		-1.0, 1.0, -1.0, 0.0, 1.0, 0.0, 0.0, -1.0,
		1.0, 1.0, -1.0, 1.0, 1.0, 0.0, 0.0, -1.0,
		-1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0, -1.0,
		1.0, -1.0, -1.0, 1.0, 0.0, 0.0, 0.0, -1.0,
		1.0, 1.0, -1.0, 1.0, 1.0, 0.0, 0.0, -1.0,
		-1.0, -1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
		-1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0,
		-1.0, -1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0,
		1.0, -1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0,
		1.0, -1.0, 1.0, 1.0, 1.0, 0.0, -1.0, 0.0,
		-1.0, -1.0, 1.0, 0.0, 1.0, 0.0, -1.0, 0.0,
		1.0, -1.0, -1.0, 1.0, 0.0, 0.0, -1.0, 0.0,
		-1.0, -1.0, 1.0, 0.0, 1.0, 0.0, -1.0, 0.0,
		-1.0, -1.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0,
		1.0, -1.0, -1.0, 1.0, 0.0, 0.0, -1.0, 0.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0,
		-1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
		1.0, 1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 0.0,
		-1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0,
		-1.0, 1.0, -1.0, 0.0, 0.0, 0.0, 1.0, 0.0,
		1.0, 1.0, -1.0, 1.0, 0.0, 0.0, 1.0, 0.0,
		-1.0, 1.0, -1.0, 0.0, 1.0, -1.0, 0.0, 0.0,
		-1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 0.0, 0.0,
		-1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
		-1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 0.0, 0.0,
		-1.0, -1.0, 1.0, 1.0, 0.0, -1.0, 0.0, 0.0,
		-1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
		1.0, 1.0, -1.0, 0.0, 1.0, 1.0, 0.0, 0.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
		1.0, -1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
		1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0,
		1.0, -1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0,
		1.0, -1.0, -1.0, 0.0, 0.0, 1.0, 0.0, 0.0
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	//Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0); //Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
	glBindVertexArray(0);             //Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO

	//Load all the proper textures
	plainTexture = importTexture("plain.jpg");
	horseSkinTexture = importTexture("horse_skin.jpg");
	grassTexture = importTexture("grass.jpg");

	//Use texture for the shadow map (the frame buffer is important for applying the shadow map before drawing the scene itself).
	unsigned int shadowMapFBO;
	glGenFramebuffers(1, &shadowMapFBO);
	unsigned int shadowMap;
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//ID's for both textures (important since shadow map shouldn't depend on the object's current texture).
	unsigned int regularTextureLoc = glGetUniformLocation(shaderProgram, "textureContent");
	unsigned int shadowMapLoc = glGetUniformLocation(shaderProgram, "shadowMap");

	//Initialize matrices required for camera and shadow map that reside in the shader program.
	transformLoc = glGetUniformLocation(shaderProgram, "model_matrix");
	viewMatrixLoc = glGetUniformLocation(shaderProgram, "view_matrix");
	projectionLoc = glGetUniformLocation(shaderProgram, "projection_matrix");
	shadowViewMatrixLoc1 = glGetUniformLocation(shaderProgram, "shadow_view_matrix");
	shadowProjectionLoc1 = glGetUniformLocation(shaderProgram, "shadow_projection_matrix");

	//Initialize matrices required for light POV in shader for the shadow map.
	shadowViewMatrixLoc2 = glGetUniformLocation(shadowShaderProgram, "shadow_view_matrix");
	shadowProjectionLoc2 = glGetUniformLocation(shadowShaderProgram, "shadow_projection_matrix");

	shadowsActiveLoc = glGetUniformLocation(shaderProgram, "shadowsActive");  //Initialize boolean that determines whether to apply shadows or not.

	srand(time(NULL)); //Prevent RNG from generating the same list of numbers each time the program is loaded.

	//Generate all horses in random positions without causing collisions from the start.
	for (int i = 0; i < HORSES; i++) {
		horses.push_back(new Horse(objectColorLocation, transformLoc, cubeVAO, drawType, i + 1));
		for (int j = 0; j < horses.size() - 1; j++) {
			if (collisionDetected(horses.at(j), horses.at(i), noDir)) {
				horses.at(i)->randomizePosition();
				j = -1;                               //Need to cycle through all horses again if a collision is found!
			}
		}
	}

	worldRotation = glm::rotate(model_matrix, worldPan, glm::vec3(0.0f, 1.0f, 0.0f)) //Applied to grid and horse for world rotation.
		*glm::rotate(model_matrix, worldTilt, glm::vec3(1.0f, 0.0f, 0.0f));

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		//Check if any events have been activiated (key pressed, mouse lmoved etc.) and call corresponding response functions
		glfwPollEvents();

		//Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);               //Clear the colorbuffer (i.e. set background color)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //Clear color bit to update colors of all objects and clear depth bit to update depth bit of all objects.

		model_matrix = glm::scale(model_matrix, glm::vec3(1.0f)); //Set a basis for coordinate measurements.

		//FOR SHADOW SHADER
		glm::mat4 shadow_view_matrix = glm::lookAt(glm::vec3(0.0f, 20.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		glm::mat4 shadow_projection_matrix = glm::perspective(PI / 2, (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, 1.0f, 25.0f);
		//glm::mat4 shadow_projection_matrix = glm::ortho(-50.0f, 50.0f, -20.0f, 20.0f, -50.0f, 50.0f);

		glUseProgram(shadowShaderProgram);
		glUniformMatrix4fv(shadowTransformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniformMatrix4fv(shadowViewMatrixLoc2, 1, GL_FALSE, glm::value_ptr(shadow_view_matrix));
		glUniformMatrix4fv(shadowProjectionLoc2, 1, GL_FALSE, glm::value_ptr(shadow_projection_matrix));

		//Have the shadow map gather the proper depth values needed.
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < HORSES; i++)
			horses.at(i)->draw();
		generateGrid(shadowShaderProgram);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//Reset to window proportions (shadow map uses different proportions).
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Vectors used for the view matrix. Ensure that viewUp is the proper vector direction relative to viewPos. Keep viewCenter at the origin.
		glm::vec3 viewPos = glm::vec3(0.0f, 20.0f, 0.0f);
		glm::vec3 viewCenter = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 viewUp = glm::vec3(0.0f, 0.0f, -1.0f);

		//Modify temporary variables used to influence the shiny part of the light.
		//Uses spherical coordinates to get x, y and z variables due to movements being rotation based.
		//x = distance*cos(theta)*sin(phi)
		//y = distance*cos(phi)             (traditionally for z-axis but y-axis is considered up axis in this context).
		//z = distance*sin(theta)*sin(phi)  (negated below due to program reversing the orientation of z-axis in comparison to natural world orientation).
		tempViewPosX = 20.0f*cos(theta)*sin(phi);
		tempViewPosY = 20.0f*cos(phi);
		tempViewPosZ = -20.0f*sin(theta)*sin(phi);

		glm::mat4 view_matrix;                                    //Dictates a camera's position and where the camera is facing.
		view_matrix = glm::lookAt(viewPos, viewCenter, viewUp)
			*glm::translate(model_matrix, viewUp)
			*glm::translate(model_matrix, (glm::cross(glm::normalize(viewPos), glm::normalize(viewUp)))
			*(translateX))
			*glm::rotate(model_matrix, cameraTilt, glm::normalize(glm::cross(viewUp, viewPos)))
			*glm::rotate(model_matrix, cameraPan, glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 projection_matrix;                              //Dictate's a camera's zoom and it's near and far planes.
		projection_matrix = glm::perspective(zoomValue, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f)*
			glm::scale(model_matrix, glm::vec3(windowAdjustmentX, windowAdjustmentY, 1.0f)); //Let's camera be adaptable to current window size (near plane is 0.1f since z-buffering
		//doesn't like near plane at 0.0f)

		//Let the program make use of model, view and projection matrices (for the camera to get the proper view and the light to get the proper shadows).
		glUseProgram(shaderProgram);
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model_matrix));
		glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(view_matrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
		glUniformMatrix4fv(shadowViewMatrixLoc1, 1, GL_FALSE, glm::value_ptr(shadow_view_matrix));
		glUniformMatrix4fv(shadowProjectionLoc1, 1, GL_FALSE, glm::value_ptr(shadow_projection_matrix));
		glUniform1i(regularTextureLoc, 0);
		glUniform1i(shadowMapLoc, 1);

		glUniform4f(lightColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);                      //Set light color (currently white).
		glUniform3f(lightPositionLocation, 0.0f, 20.0f, 0.0f);                        //Set light position (currently 20 units above initial horse position).
		glUniform3f(viewPositionLocation, tempViewPosX, tempViewPosY, tempViewPosZ);  //Use temporary camera position variables to influence specular lighting.
		glUniform1i(shadowsActiveLoc, shadowsActive);                                 //Indicate to shader whether to apply shadows or not.

		glActiveTexture(GL_TEXTURE1);                                                 //Bind shadow map to proper texture ID.
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glActiveTexture(GL_TEXTURE0);                                                 //Allow actual textures to be binded to proper texture ID.
		if (texturesActive)                                                           //Use horse skin texture if textures are active. Otherwise, use plain texture.
			glBindTexture(GL_TEXTURE_2D, horseSkinTexture);
		else
			glBindTexture(GL_TEXTURE_2D, plainTexture);
		for (int i = 0; i < HORSES; i++)
			horses.at(i)->draw();                                                     //Render horse.
		glActiveTexture(GL_TEXTURE0);
		if (texturesActive)                                                           //Use grass texture if textures are active. Otherwise, use plain texture.
			glBindTexture(GL_TEXTURE_2D, grassTexture);
		else
			glBindTexture(GL_TEXTURE_2D, plainTexture);
		generateGrid(shaderProgram);                                                  //Render floor.

		//Collision detection loop. Accounts for entry of collision, during the collision and once the collision ends.
		for (int i = 0; i < HORSES - 1; i++)
			for (int j = i + 1; j < HORSES; j++) {
				if (collisionDetected(horses.at(i), horses.at(j), straightDir))
					collisionResolutionDuring(horses.at(i), horses.at(j));
				else
					collisionResolutionEnd(horses.at(i), horses.at(j));
			}

		//Reset various properties to allow collision detection to resume as normal next frame.
		for (int i = 0; i < HORSES - 1; i++) {
			//If two horses collided with each other and are in a stopped state, allow one of them to avoid so they aren't permanently stuck.
			if (horses.at(i)->getCollisionStatus() != normal && horses.at(i)->doCollisionsExist()) {
				Horse* currentHorse = horses.at(i);
				Horse* otherHorse = horses.at(horses.at(i)->getCurrentCollision() - 1);
				if (horses.at(i)->getCollisionStatus() == stopped && otherHorse->getCollisionStatus() == stopped) {
					randomNumber(0, 1) == 0 ? currentHorse->setCollisionStatus(avoiding) : otherHorse->setCollisionStatus(avoiding);
				}
			}
			if (horses.at(i)->getDirectionAssigned() == true) //Ensure that an avoiding horse is not stuck with left or right in subsequent
				horses.at(i)->setDirectionAssigned(false);    //collision checks.
		}

		//Updates position and animation of horse. Only accessed when animations are on.
		if (animationActive)
			for (int i = 0; i < HORSES; i++)
				horses.at(i)->updatePosition();

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}

//Is called whenever a key is pressed (this specifically gets the ASCII value of the character rather
//than glfw's key value). This is used to be able to map different input onto both uppercase WASD and
//lowercase WASD.
void character_callback(GLFWwindow* window, unsigned int codepoint)
{
	if (animationActive) { //Is the animation active? If so...
		//Horse movement.
		//Uses ASCII Notation - 65/97: A/a, 68/100: D/d, 87/119: W/w, 32: Space.
		if (codepoint == 65 || codepoint == 97) {        //Rotate horse left.
			if (controllingHorse) {
				horses.at(selectedHorse - 1)->move(leftDir);
			}
		}
		if (codepoint == 68 || codepoint == 100) {       //Rotate horse right.
			if (controllingHorse) {
				horses.at(selectedHorse - 1)->move(rightDir);
			}
		}
		if (codepoint == 87 || codepoint == 119) {       //Move horse straight (if it doesn't result in a collision).
			for (int i = 0; i < HORSES; i++)
			{
				if (horses.at(selectedHorse - 1)->getId() != horses.at(i)->getId())
					if (collisionDetectedWithControlledHorse(horses.at(selectedHorse - 1), horses.at(i))) {
						break;
					}

				if (i == HORSES - 1)
					horses.at(selectedHorse - 1)->move(straightDir);
			}
		}

		//Change horse speed.
		//Uses ASCII Notation - 85/117: U/u, 74/106: J/j.
		if (codepoint == 85 || codepoint == 117) {  //Increase speed
			if (controllingHorse) {
				horses.at(selectedHorse - 1)->incrementSpeed();
			}
		}
		if (codepoint == 74 || codepoint == 106) {  //Decrease speed
			if (controllingHorse) {
				horses.at(selectedHorse - 1)->decrementSpeed();
			}
		}
	}
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)   //Close Program.
		glfwSetWindowShouldClose(window, GL_TRUE);

	//These controls influence camera rotation.
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {   //Rotate camera leftwards.
		worldPan -= PI * 2 / 180;
		updateWorldOrientation();
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){   //Rotate camera rightwards.
		worldPan += PI * 2 / 180;
		updateWorldOrientation();
	}
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) {     //Rotate camera upwards.
		worldTilt += PI * 2 / 180;
		updateWorldOrientation();
	}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {   //Rotate camera downwards.
		worldTilt -= PI * 2 / 180;
		updateWorldOrientation();
	}

	//Set camera back to initial position.
	if (key == GLFW_KEY_HOME && action == GLFW_PRESS) {
		cameraPan = 0.0f;
		cameraTilt = 0.0f;
		translateX = 0.0f;
		worldPan = 0.0f;
		worldTilt = 0.0f;
		zoomValue = PI / 4;
		phi = 0;
		theta = 3 * PI / 2;
		updateWorldOrientation();
	}

	//Change how scene is rendered (full model with triangles, mesh model with lines or index representation with points)
	if (key == GLFW_KEY_T && action == GLFW_PRESS) { //Triangles
		drawType = GL_TRIANGLES;
		indiceQuantity = 6;                          //6 points to traverse with triangles.
		updateDrawType();
	}
	if (key == GLFW_KEY_L && action == GLFW_PRESS) { //Lines
		drawType = GL_LINES;
		indiceQuantity = 10;                         //10 points to traverse with lines.
		updateDrawType();
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) { //Points
		drawType = GL_POINTS;
		indiceQuantity = 10;
		updateDrawType();
	}

	//Toggle textures for scene.
	if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		if (texturesActive)
			texturesActive = false;
		else
			texturesActive = true;
	}

	//Toggle shadows for scene.
	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		if (shadowsActive)
			shadowsActive = false;
		else
			shadowsActive = true;
	}

	//Toggle whether to debug collisions or not.
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		if (debugCollisions) {
			debugCollisions = false;
			for (int i = 0; i < HORSES; i++) {
				horses.at(i)->setDebugCollisionStatus(false);
				horses.at(i)->updateDebugColors();
			}
		}
		else {
			debugCollisions = true;
			for (int i = 0; i < HORSES; i++) {
				horses.at(i)->setDebugCollisionStatus(true);
				horses.at(i)->updateDebugColors();
			}
		}
	}

	//Toggle animations for all horses.
	if (key == GLFW_KEY_H && action == GLFW_PRESS) {
		if (animationActive)
			animationActive = false;
		else
			animationActive = true;
	}

	//Allow user to cycle leftwards through horses when selecting a horse.
	if (key == GLFW_KEY_A && action == GLFW_PRESS) {
		if (selectingHorse) {
			horses.at(selectedHorse - 1)->setIsSelected(false);
			selectedHorse--;
			if (selectedHorse < 1)
				selectedHorse = HORSES;
			horses.at(selectedHorse - 1)->setIsSelected(true);
		}
	}

	//Allow user to cycle rightwards through horses when selecting a horse.
	if (key == GLFW_KEY_D && action == GLFW_PRESS) {
		if (selectingHorse) {
			horses.at(selectedHorse - 1)->setIsSelected(false);
			selectedHorse++;
			if (selectedHorse > HORSES)
				selectedHorse = 1;
			horses.at(selectedHorse - 1)->setIsSelected(true);
		}
	}

	//Allow user to select a horse/give up control of a horse.
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS){
		if (selectingHorse) {
			horses.at(selectedHorse - 1)->setIsControlled(true);
			horses.at(selectedHorse - 1)->setIsSelected(false);
			controllingHorse = true;
			selectingHorse = false;
		}
		else if (controllingHorse) {
			horses.at(selectedHorse - 1)->setIsControlled(false);
			controllingHorse = false;
		}
	}

	//Allow user to stop a horse (the horse jumps when stopped)
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && animationActive) {
		if (controllingHorse && !horses.at(selectedHorse - 1)->getIsHorseStopped()) {
			horses.at(selectedHorse - 1)->stopHorse();
		}
	}

	//Hold this to access the horse selection menu
	if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
	{
		if (!controllingHorse) {
			if (action == GLFW_PRESS) {
				horses.at(selectedHorse - 1)->setIsSelected(true);
				selectingHorse = true;
			}
			if (action == GLFW_RELEASE) {
				horses.at(selectedHorse - 1)->setIsSelected(false);
				selectingHorse = false;
			}
		}
	}
}

//Called when any three mouse buttons are pressed (this provides important input required for cursor callback function).
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	//If we click any mouse button... (and no other mouse buttons are already being held!)
	if ((button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE || button == GLFW_MOUSE_BUTTON_RIGHT)
		&& action == GLFW_PRESS && !middleMouseHold) {
		recordStartingPos = true;                 //Indicate that we need to record the cursor position when we clicked a mouse button.
		if (button == GLFW_MOUSE_BUTTON_LEFT)     //Indicate which mouse button is being currently held...
			leftMouseHold = true;
		if (button == GLFW_MOUSE_BUTTON_MIDDLE)
			middleMouseHold = true;
		if (button == GLFW_MOUSE_BUTTON_RIGHT)
			rightMouseHold = true;
	}

	//Indicate that the current mouse button beind held is now no longer held...
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		leftMouseHold = false;
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
		middleMouseHold = false;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		rightMouseHold = false;
}

//Called when the mouse cursor is moved.
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (recordStartingPos) {        //Record starting position when a mouse button is clicked...
		previousXPos = xpos;
		previousYPos = ypos;
		recordStartingPos = false;
	}
	else {                          //Constantly keep track of current and previous mouse position.
		previousXPos = currentXPos;
		previousYPos = currentYPos;
		currentXPos = xpos;
		currentYPos = ypos;
	}

	//Store variables calculating how far the mouse is moved so it can provide the mouse-based camera movements with
	//proper functionality.
	xPosDifference = currentXPos - previousXPos;
	yPosDifference = currentYPos - previousYPos;

	//Check which mouse button is currently being held. Use difference in y-position to influence zoom and tilt and use
	//difference in x-position to influcence rotation around y-axis.
	//Middle mouse button or Left/Right mouse buttons Simultaneously: Tilt
	//Left mouse button: Zoom
	//Right mouse button: Pan
	if (middleMouseHold || (leftMouseHold && rightMouseHold)) {
		cameraTilt -= (float)yPosDifference / 100;
		phi += (float)yPosDifference / 100;
	}
	else if (leftMouseHold) {
		zoomValue += (float)yPosDifference / 1000;
		if (zoomValue < 0.1f)        //Don't attempt to zoom "through" stuff (doing that results in inverted lens).
			zoomValue = 0.1f;
		if (zoomValue > 7 * PI / 8)  //Set zoom out limitation so the lens don't get inverted due to excess zooming.
			zoomValue = 7 * PI / 8;
	}
	else if (rightMouseHold) {
		cameraPan -= (float)xPosDifference / 100;
		theta += (float)xPosDifference / 100;
	}
}

//Called when user resizes window.
void window_size_callback(GLFWwindow* window, int newWidth, int newHeight)
{
	glViewport(0, 0, newWidth, newHeight);                  //Indicate to program to adapt to new width and height.
	windowAdjustmentX = INITIAL_WIDTH / (float)newWidth;    //Adjust the horizontal scale of the camera to counterbalance window change (so it doesn't squish).
	windowAdjustmentY = INITIAL_HEIGHT / (float)newHeight;  //Adjust the vertical scale of the camera to counterbalance window change (so it doesn't squish).
	WIDTH = newWidth;
	HEIGHT = newHeight;
}

//Called so that each horse is rendered the specified way when the user changes the rendering type.
void updateDrawType()
{
	for (int i = 0; i < HORSES; i++)
		horses.at(i)->setDrawType(drawType);
}

//Update world orientation for both grid and all horses.
void updateWorldOrientation()
{
	worldRotation = glm::rotate(model_matrix, worldPan, glm::vec3(0.0f, 1.0f, 0.0f)) //Applied to grid and horse for world rotation.
		* glm::rotate(model_matrix, worldTilt, glm::vec3(1.0f, 0.0f, 0.0f));

	for (int i = 0; i < HORSES; i++)
		horses.at(i)->setWorldRotation(worldRotation);
}

GLuint importShaders(string vertex_shader_path, string fragment_shader_path)
{
	//Build and compile our shader program

	//Vertex shader
	//Read the Vertex Shader code from the file
	string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_shader_path, ios::in);

	if (VertexShaderStream.is_open()) {
		string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory ?\n", vertex_shader_path.c_str());
		getchar();
		exit(-1);
	}

	//Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);

	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}
	else {
		printf("Impossible to open %s. Are you in the right directory?\n", fragment_shader_path.c_str());
		getchar();
		exit(-1);
	}

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(vertexShader, 1, &VertexSourcePointer, NULL);
	glCompileShader(vertexShader);

	//Check for compile time errors
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(fragmentShader, 1, &FragmentSourcePointer, NULL);
	glCompileShader(fragmentShader);

	//Check for compile time errors
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	//Link shaders
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	//Check for linking errors
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader); //free up memory
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

unsigned int importTexture(char const *file_path)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// load and generate the plain texture
	int width, height, nrChannels;
	unsigned char *textureData = stbi_load(file_path, &width, &height, &nrChannels, 0);
	if (textureData)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);  //Potential fix here...
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(textureData);
	return texture;
}

//Used for ongoing collisions.
float distanceBetweenTwoPoints(float x1, float x2, float y1, float y2, float z1, float z2) {
	return sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1) + (z2 - z1)*(z2 - z1));
}

//Basic collision detection where if a horse's sphere is in another, they are collided.
//Verified by checking if distance is smaller than the sum of the radii of each horse.
bool sphereCollisionDetection(glm::vec3 pos1, glm::vec3 pos2, float radius1, float radius2)
{
	float distance = sqrt((pos2.x - pos1.x)*(pos2.x - pos1.x)
		+ (pos2.y - pos2.y)*(pos2.y - pos2.y)
		+ (pos2.z - pos1.z)*(pos2.z - pos1.z));
	if (distance <= radius1 + radius2)
		return true;
	else
		return false;
}

//Check if two horses would collide with each other in the next frame.
bool collisionDetected(Horse* horse1, Horse* horse2, forecastDirection direction)
{
	glm::vec3 horseVec1 = horse1->getForecastedPosition(direction);
	glm::vec3 horseVec2 = horse2->getForecastedPosition(direction);
	float horseRadius1 = horse1->getCollisionRadius();
	float horseRadius2 = horse2->getCollisionRadius();
	return sphereCollisionDetection(horseVec1, horseVec2, horseRadius1, horseRadius2);
}

//Checks if a controlled horse and an independent horse would collide with each other in the next frame.
bool collisionDetectedWithControlledHorse(Horse* controlledHorse, Horse* independentHorse)
{
	glm::vec3 horseVecControlled = controlledHorse->getForecastedPosition(straightDir);
	glm::vec3 horseVecIndependent = independentHorse->getForecastedPosition(noDir);
	float horseRadiusControlled = controlledHorse->getCollisionRadius();
	float horseRadiusIndependent = independentHorse->getCollisionRadius();
	return sphereCollisionDetection(horseVecControlled, horseVecIndependent, horseRadiusControlled, horseRadiusIndependent);
}

//Checks if horses are farther away from each other if the avoided horse goes straight.
//This is done by checking if the distance calculated with no horse movement is smaller than the distance
//calculated accounting for if the avoiding horse moves straight next frame
bool isFartherFromCollision(Horse* stoppedHorse, Horse* avoidingHorse, forecastDirection direction)
{
	glm::vec3 horseVecStopped = stoppedHorse->getForecastedPosition(noDir);
	glm::vec3 horseVecAvoidingBefore = avoidingHorse->getForecastedPosition(noDir);
	glm::vec3 horseVecAvoidingAfter = avoidingHorse->getForecastedPosition(direction);
	float distanceBefore = distanceBetweenTwoPoints(horseVecStopped.x, horseVecAvoidingBefore.x, horseVecStopped.y, horseVecAvoidingBefore.y, horseVecStopped.z, horseVecAvoidingBefore.z);
	float distanceAfter = distanceBetweenTwoPoints(horseVecStopped.x, horseVecAvoidingAfter.x, horseVecStopped.y, horseVecAvoidingAfter.y, horseVecStopped.z, horseVecAvoidingAfter.z);
	if (distanceBefore <= distanceAfter)
		return true;
	else
		return false;
}

//Check if a horse is going out of bounds.
bool goingOutOfBounds(Horse* avoidingHorse)
{
	glm::vec3 horseVec = avoidingHorse->getForecastedPosition(straightDir);
	if ((horseVec.x >= -50.0f && horseVec.x <= 50.0f) && (horseVec.z >= -50.0f && horseVec.z <= 50.0f))
		return false;
	else
		return true;
}

//The main collision resolution loop. Accounts for the following scenarios:
//- Two normal horses collided.
//- A normal horse and a collided horse collide.
//- If two avoiding horses collide.
//- Collision of independent horse and controlled horse.
//- Collision of an avoiding horse and a stopped horse.
void collisionResolutionDuring(Horse* horse1, Horse* horse2) {
	//Two normal horses: Set one to avoid and the other to stop.
	if (horse1->getCollisionStatus() == normal && horse2->getCollisionStatus() == normal) {
		horse1->setCollisionStatus(randomNumber(0, 1) == 0 ? stopped : avoiding);
		horse2->setCollisionStatus(horse1->getCollisionStatus() == stopped ? avoiding : stopped);
	}
	//A normal horse and a collided horse: Set the normal horse to stop.
	else if (horse1->getCollisionStatus() == normal || horse2->getCollisionStatus() == normal)
	{
		Horse* normalHorse;
		Horse* collidedHorse;
		horse1->getCollisionStatus() == normal ? (normalHorse = horse1, collidedHorse = horse2) : (normalHorse = horse2, collidedHorse = horse1);
		normalHorse->setCollisionStatus(stopped);
	}
	//Two avoiding horses: Randomly select one to stop.
	else if (horse1->getCollisionStatus() == avoiding && horse2->getCollisionStatus() == avoiding) {
		horse1->setCollisionStatus(randomNumber(0, 1) == 0 ? stopped : avoiding);
		horse2->setCollisionStatus(horse1->getCollisionStatus() == stopped ? avoiding : stopped);
	}
	//Scenario when one horse is controlled by the user. Independent horse starts avoiding in this type of collision.
	else if (horse1->getCollisionStatus() == controlled || horse2->getCollisionStatus() == controlled) {
		Horse* controlledHorse;
		Horse* independentHorse;
		horse1->getCollisionStatus() == controlled ?
			(horse2->setCollisionStatus(avoiding), controlledHorse = horse1, independentHorse = horse2) :
			(horse1->setCollisionStatus(avoiding), controlledHorse = horse2, independentHorse = horse1);
		//Avoiding horse should go straight if:
		//- It goes farther from the collision
		//- It doesn't go out of bounds
		//- Previous collision checks for the avoiding horse doesn't tell it to go left or right.
		if (isFartherFromCollision(controlledHorse, independentHorse, straightDir)
			&& !goingOutOfBounds(independentHorse)
			&& (independentHorse->getDirectionAssigned() == false || (independentHorse->getDirectionAssigned() == true && independentHorse->getAvoidingDirection() == straightDir)))
			independentHorse->setAvoidingDirection(straightDir);
		//Avoiding horse randomly decide to go left or right if not already assigned a direction (so horse doesn't alternate between left and right randomly).
		else if (independentHorse->getAvoidingDirection() != leftDir && independentHorse->getAvoidingDirection() != rightDir) {
			if (randomNumber(0, 1) == 0) {
				if (isFartherFromCollision(independentHorse, independentHorse, leftDir))
					independentHorse->setAvoidingDirection(leftDir);
				else
					independentHorse->setAvoidingDirection(rightDir);
			}
			else {
				if (isFartherFromCollision(independentHorse, independentHorse, rightDir))
					independentHorse->setAvoidingDirection(rightDir);
				else
					independentHorse->setAvoidingDirection(leftDir);
			}
		}
		//Indicate that horse was assigned a direction.
		if (independentHorse->getDirectionAssigned() == false)
			independentHorse->setDirectionAssigned(true);
	}
	else if (horse1->getCollisionStatus() != normal && horse2->getCollisionStatus() != normal)
	{
		Horse* stoppedHorse;
		Horse* avoidingHorse;
		horse1->getCollisionStatus() == stopped ?
			(stoppedHorse = horse1, avoidingHorse = horse2) :
			(stoppedHorse = horse2, avoidingHorse = horse1);
		//Avoiding horse should go straight if:
		//- It goes farther from the collision
		//- It doesn't go out of bounds
		//- Previous collision checks for the avoiding horse doesn't tell it to go left or right.
		if (isFartherFromCollision(stoppedHorse, avoidingHorse, straightDir)
			&& !goingOutOfBounds(avoidingHorse)
			&& (avoidingHorse->getDirectionAssigned() == false || (avoidingHorse->getDirectionAssigned() == true && avoidingHorse->getAvoidingDirection() == straightDir)))
			avoidingHorse->setAvoidingDirection(straightDir);
		//Avoiding horse randomly decides to go left or right if not already assigned a direction (so horse doesn't alternate between left and right randomly).
		else if (avoidingHorse->getAvoidingDirection() != leftDir && avoidingHorse->getAvoidingDirection() != rightDir) {
			if (randomNumber(0, 1) == 0) {
				if (isFartherFromCollision(stoppedHorse, avoidingHorse, leftDir))
					avoidingHorse->setAvoidingDirection(leftDir);
				else
					avoidingHorse->setAvoidingDirection(rightDir);
			}
			else {
				if (isFartherFromCollision(stoppedHorse, avoidingHorse, rightDir))
					avoidingHorse->setAvoidingDirection(rightDir);
				else
					avoidingHorse->setAvoidingDirection(leftDir);
			}
		}
		//Indicate that horse was assigned a direction.
		if (avoidingHorse->getDirectionAssigned() == false)
			avoidingHorse->setDirectionAssigned(true);
		//If a horse has turned 360 degrees (in general, not entirely left or right), we assume it's trapped and set it free.
		//We do this by giving a horse that is collided with the avoiding horse avoiding behaviour while the other horse gets stopped
		//behaviour. The one who collided with the avoided horse first gets avoid behaviour.
		if (avoidingHorse->isTrapped()) {
			avoidingHorse->setCollisionStatus(stopped);
			avoidingHorse->setDirectionAssigned(false);
			avoidingHorse->setAvoidingDirection(noDir);
			Horse* newAvoidingHorse = horses.at(avoidingHorse->getCurrentCollision() - 1);
			newAvoidingHorse->setCollisionStatus(avoiding);
		}

	}
	//Update collision list of each horse.
	if (!horse1->collisionTargetPresent(horse2->getId())) {
		horse1->addCollision(horse2->getId());
		horse2->addCollision(horse1->getId());
	}
}

//Update collision properties for the horses if they are just getting out of a collision.
void collisionResolutionEnd(Horse* horse1, Horse* horse2) {
	Horse* stoppedHorse;
	Horse* avoidingHorse;
	horse1->getCollisionStatus() == stopped ?
		(stoppedHorse = horse1, avoidingHorse = horse2) :
		(stoppedHorse = horse2, avoidingHorse = horse1);
	if (horse1->collisionTargetPresent(horse2->getId()))
		horse1->removeCollision(horse2->getId());
	if (horse2->collisionTargetPresent(horse1->getId()))
		horse2->removeCollision(horse1->getId());
	if (!horse1->doCollisionsExist()) {
		horse1->setCollisionStatus(normal);
		if (horse1->getAvoidingDirection() != noDir)
			horse1->setAvoidingDirection(noDir);
	}
	if (!horse2->doCollisionsExist()) {
		horse2->setCollisionStatus(normal);
		if (horse2->getAvoidingDirection() != noDir)
			horse2->setAvoidingDirection(noDir);
	}
}

//Generate the floor of the scene.
void generateGrid(GLuint shaderProgram)
{
	glm::mat4 instance; //Variable that contains the transformation parameters of the current grid line.

	//Set grid color to white.
	glUseProgram(shaderProgram);
	glUniform4f(objectColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

	//Create the grid.
	glBindVertexArray(gridVAO);
	instance = worldRotation;
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(instance));
	glDrawArrays(drawType, 0, indiceQuantity);
	glBindVertexArray(0);
}

//Generates random integer from min to max
int randomNumber(int min, int max)
{
	return rand() % (max - min + 1) + min;
}

