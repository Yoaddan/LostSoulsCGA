#define _USE_MATH_DEFINES
#include <cmath>
//glew include
#include <GL/glew.h>

//std includes
#include <string>
#include <iostream>

//glfw include
#include <GLFW/glfw3.h>

// program include
#include "Headers/TimeManager.h"

// Shader include
#include "Headers/Shader.h"

// Model geometric includes
#include "Headers/Sphere.h"
#include "Headers/Cylinder.h"
#include "Headers/Box.h"
#include "Headers/FirstPersonCamera.h"
#include "Headers/ThirdPersonCamera.h"

// Font rendering include
#include "Headers/FontTypeRendering.h"

//GLM include
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Headers/Texture.h"

// Include loader Model class
#include "Headers/Model.h"

// Include Terrain
#include "Headers/Terrain.h"

#include "Headers/AnimationUtils.h"

// Include Colision headers functions
#include "Headers/Colisiones.h"

// OpenAL include
#include <AL/alut.h>

// Modelo dinamico de una caja para las sombras
#include "Headers/ShadowBox.h"

#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))

int screenWidth;
int screenHeight;

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

GLFWwindow *window;

Shader shader;
//Shader con skybox
Shader shaderSkybox;
//Shader con multiples luces
Shader shaderMulLighting;
//Shader para el terreno
Shader shaderTerrain;
// Shader para dibujar un objeto con solo textura
Shader shaderTexture;
// Shader para el render del buffer de profundidad
Shader shaderDepth;
// Shader para sombra
Shader shaderViewDepth;

std::shared_ptr<Camera> camera(new ThirdPersonCamera());
float distanceFromTarget = 7.0;

Sphere skyboxSphere(20, 20);
Box boxWalls;
Box boxHighway;
Box boxLandingPad;
Sphere esfera1(10, 10);
Box boxCollider;
Sphere sphereCollider(10, 10);
Cylinder rayModel(10, 10, 1.0, 1.0, 1.0);
Box boxIntro;
Box boxViewDepth;

// Modelos externos
Model laberinto; //Laberinto principal.
Model fantasma; //Enemigo que atraviesa paredes.
Model guardia; //Enemigo con ruta predefinida.

// Lamps
Model modelLamp1;
Model modelLamp2;
Model modelLampPost2;

// Personaje principal
// Mayow
Model mayowModelAnimate;

// Terrain model instance
Terrain terrain(-1, -1, 200, 5, "../Textures/Height Map/heightmap.png");

// Apuntador shadow box
ShadowBox * shadowBox;

GLuint textureTerrainRID, textureTerrainGID, textureTerrainBID, textureTerrainBlendMapID;
GLuint skyboxTextureID;
GLuint textureInit1ID, textureInit2ID, textureActivaID, textureScreenID;

bool iniciaPartida = false, presionarOpcion = false;

// Modelo para el render del texto
FontTypeRendering::FontTypeRendering *modelText;

GLenum types[6] = {
GL_TEXTURE_CUBE_MAP_POSITIVE_X,
GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };

std::string fileNames[6] = { "../Textures/Skybox/front.png",
		"../Textures/Skybox/back.png",
		"../Textures/Skybox/up.png",
		"../Textures/Skybox/down.png",
		"../Textures/Skybox/right.png",
		"../Textures/Skybox/left.png" };

bool exitApp = false;
int lastMousePosX, offsetX = 0;
int lastMousePosY, offsetY = 0;

// Model matrix definitions
glm::mat4 modelMatrixLaberinto = glm::mat4(1.0f);
glm::mat4 modelMatrixFantasma = glm::mat4(1.0f);
glm::mat4 modelMatrixGuardia = glm::mat4(1.0f);

glm::mat4 modelMatrixMayow = glm::mat4(1.0f);

int animationMayowIndex = 1;
int modelSelected = 0;
bool enableCountSelected = true;

// Lamps position
std::vector<glm::vec3> lamp1Position = {
	glm::vec3(-7.03, 0, -19.14),
	glm::vec3(24.41, 0, -34.57),
	glm::vec3(-10.15, 0, -54.1)
};
std::vector<float> lamp1Orientation = {
	-17.0, -82.67, 23.70
};
std::vector<glm::vec3> lamp2Position = {
	glm::vec3(-36.52, 0, -23.24),
	glm::vec3(-52.73, 0, -3.90)
};
std::vector<float> lamp2Orientation = {
	21.37 + 90, -65.0 + 90
};

double deltaTime;
double currTime, lastTime;

// Colliders
std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> > collidersOBB;
std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> > collidersSBB;

// Variables maquina de estados

// OpenAL Defines
#define NUM_BUFFERS 3
#define NUM_SOURCES 3
#define NUM_ENVIRONMENTS 1
// Listener
ALfloat listenerPos[] = { 0.0, 0.0, 4.0 };
ALfloat listenerVel[] = { 0.0, 0.0, 0.0 };
ALfloat listenerOri[] = { 0.0, 0.0, 1.0, 0.0, 1.0, 0.0 };
// Source 0
ALfloat source0Pos[] = { -2.0, 0.0, 0.0 };
ALfloat source0Vel[] = { 0.0, 0.0, 0.0 };
// Source 1
ALfloat source1Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source1Vel[] = { 0.0, 0.0, 0.0 };
// Source 2
ALfloat source2Pos[] = { 2.0, 0.0, 0.0 };
ALfloat source2Vel[] = { 0.0, 0.0, 0.0 };
// Buffers
ALuint buffer[NUM_BUFFERS];
ALuint source[NUM_SOURCES];
ALuint environment[NUM_ENVIRONMENTS];
// Configs
ALsizei size, freq;
ALenum format;
ALvoid *data;
int ch;
ALboolean loop;
std::vector<bool> sourcesPlay = {true, true, true};

// Framesbuffers
GLuint depthMap, depthMapFBO;

// Se definen todos las funciones.
void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes);
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
		int mode);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void init(int width, int height, std::string strTitle, bool bFullScreen);
void destroy();
bool processInput(bool continueApplication = true);

// Implementacion de todas las funciones.
void init(int width, int height, std::string strTitle, bool bFullScreen) {

	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		exit(-1);
	}

	screenWidth = width;
	screenHeight = height;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	if (bFullScreen)
		window = glfwCreateWindow(width, height, strTitle.c_str(),
				glfwGetPrimaryMonitor(), nullptr);
	else
		window = glfwCreateWindow(width, height, strTitle.c_str(), nullptr,
				nullptr);

	if (window == nullptr) {
		std::cerr
				<< "Error to create GLFW window, you can try download the last version of your video card that support OpenGL 3.3+"
				<< std::endl;
		destroy();
		exit(-1);
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetWindowSizeCallback(window, reshapeCallback);
	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Init glew
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Failed to initialize glew" << std::endl;
		exit(-1);
	}

	glViewport(0, 0, screenWidth, screenHeight);
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// Inicialización de los shaders
	shader.initialize("../Shaders/colorShader.vs", "../Shaders/colorShader.fs");
	shaderSkybox.initialize("../Shaders/skyBox.vs", "../Shaders/skyBox_fog.fs");
	shaderMulLighting.initialize("../Shaders/iluminacion_textura_animation_shadow.vs", "../Shaders/multipleLights_shadow.fs");
	shaderTerrain.initialize("../Shaders/terrain_shadow.vs", "../Shaders/terrain_shadow.fs");
	shaderTexture.initialize("../Shaders/texturizado.vs", "../Shaders/texturizado.fs");
	shaderDepth.initialize("../Shaders/shadow_mapping_depth.vs", "../Shaders/shadow_mapping_depth.fs");
	shaderViewDepth.initialize("../Shaders/texturizado.vs", "../Shaders/texturizado_depth_view.fs");

	// Inicializacion de los objetos.
	skyboxSphere.init();
	skyboxSphere.setShader(&shaderSkybox);
	skyboxSphere.setScale(glm::vec3(20.0f, 20.0f, 20.0f));

	boxCollider.init();
	boxCollider.setShader(&shader);
	boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

	sphereCollider.init();
	sphereCollider.setShader(&shader);
	sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));

	rayModel.init();
	rayModel.setShader(&shader);
	rayModel.setColor(glm::vec4(1.0));

	boxIntro.init();
	boxIntro.setShader(&shaderTexture);
	boxIntro.setScale(glm::vec3(2.0, 2.0, 1.0));

	boxViewDepth.init();
	boxViewDepth.setShader(&shaderViewDepth);

	// Laberinto
	laberinto.loadModel("../models/Laberinto/Laberinto.obj");
	laberinto.setShader(&shaderMulLighting);

	// Fantasma
	fantasma.loadModel("../models/Fantasma/Fantasma.obj");
	fantasma.setShader(&shaderMulLighting);

	// Guardia
	guardia.loadModel("../models/Guardia/c1530.obj");
	guardia.setShader(&shaderMulLighting);

	//Lamps models
	modelLamp1.loadModel("../models/Street-Lamp-Black/objLamp.obj");
	modelLamp1.setShader(&shaderMulLighting);
	modelLamp2.loadModel("../models/Street_Light/Lamp.obj");
	modelLamp2.setShader(&shaderMulLighting);
	modelLampPost2.loadModel("../models/Street_Light/LampPost.obj");
	modelLampPost2.setShader(&shaderMulLighting);

	// Mayow
	mayowModelAnimate.loadModel("../models/mayow/personaje2.fbx");
	mayowModelAnimate.setShader(&shaderMulLighting);

	// Terreno
	terrain.init();
	terrain.setShader(&shaderTerrain);

	// Se inicializa el model de render text
	modelText = new FontTypeRendering::FontTypeRendering(screenWidth, screenHeight);
	modelText->Initialize();

	camera->setPosition(glm::vec3(0.0, 3.0, 4.0));
	camera->setDistanceFromTarget(distanceFromTarget);
	camera->setSensitivity(1.0);
	
	// Carga de texturas para el skybox
	Texture skyboxTexture = Texture("");
	glGenTextures(1, &skyboxTextureID);
	// Tipo de textura CUBE MAP
	glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTextureID);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	for (int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(types); i++) {
		skyboxTexture = Texture(fileNames[i]);
		skyboxTexture.loadImage(true);
		if (skyboxTexture.getData()) {
			glTexImage2D(types[i], 0, skyboxTexture.getChannels() == 3 ? GL_RGB : GL_RGBA, skyboxTexture.getWidth(), skyboxTexture.getHeight(), 0,
			skyboxTexture.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, skyboxTexture.getData());
		} else
			std::cout << "Failed to load texture" << std::endl;
		skyboxTexture.freeImage();
	}

	// Defininiendo texturas del mapa de mezclas
	// Definiendo la textura
	Texture textureR("../Textures/Terrain/rojo.png");
	textureR.loadImage(); // Cargar la textura
	glGenTextures(1, &textureTerrainRID); // Creando el id de la textura
	glBindTexture(GL_TEXTURE_2D, textureTerrainRID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureR.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureR.getChannels() == 3 ? GL_RGB : GL_RGBA, textureR.getWidth(), textureR.getHeight(), 0,
		textureR.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureR.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureR.freeImage(); // Liberamos memoria

	// Definiendo la textura
	Texture textureG("../Textures/Terrain/verde.png");
	textureG.loadImage(); // Cargar la textura
	glGenTextures(1, &textureTerrainGID); // Creando el id de la textura
	glBindTexture(GL_TEXTURE_2D, textureTerrainGID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureG.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureG.getChannels() == 3 ? GL_RGB : GL_RGBA, textureG.getWidth(), textureG.getHeight(), 0,
		textureG.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureG.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureG.freeImage(); // Liberamos memoria

	// Definiendo la textura
	Texture textureB("../Textures/Terrain/azul.png");
	textureB.loadImage(); // Cargar la textura
	glGenTextures(1, &textureTerrainBID); // Creando el id de la textura
	glBindTexture(GL_TEXTURE_2D, textureTerrainBID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureB.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureB.getChannels() == 3 ? GL_RGB : GL_RGBA, textureB.getWidth(), textureB.getHeight(), 0,
		textureB.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureB.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureB.freeImage(); // Liberamos memoria

	// Definiendo la textura
	Texture textureBlendMap("../Textures/Blend Map/blendmap.png");
	textureBlendMap.loadImage(); // Cargar la textura
	glGenTextures(1, &textureTerrainBlendMapID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureTerrainBlendMapID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureBlendMap.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureBlendMap.getChannels() == 3 ? GL_RGB : GL_RGBA, textureBlendMap.getWidth(), textureBlendMap.getHeight(), 0,
		textureBlendMap.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureBlendMap.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureBlendMap.freeImage(); // Liberamos memoria

	// Definiendo la textura
	Texture textureIntro1("../Textures/Intro1.png");
	textureIntro1.loadImage(); // Cargar la textura
	glGenTextures(1, &textureInit1ID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureInit1ID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureIntro1.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureIntro1.getChannels() == 3 ? GL_RGB : GL_RGBA, textureIntro1.getWidth(), textureIntro1.getHeight(), 0,
		textureIntro1.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureIntro1.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureIntro1.freeImage(); // Liberamos memoria

	// Definiendo la textura
	Texture textureIntro2("../Textures/Intro2.png");
	textureIntro2.loadImage(); // Cargar la textura
	glGenTextures(1, &textureInit2ID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureInit2ID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureIntro2.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureIntro2.getChannels() == 3 ? GL_RGB : GL_RGBA, textureIntro2.getWidth(), textureIntro2.getHeight(), 0,
		textureIntro2.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureIntro2.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureIntro2.freeImage(); // Liberamos memoria

	// Definiendo la textura
	Texture textureScreen("../Textures/Screen.png");
	textureScreen.loadImage(); // Cargar la textura
	glGenTextures(1, &textureScreenID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureScreenID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureScreen.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureScreen.getChannels() == 3 ? GL_RGB : GL_RGBA, textureScreen.getWidth(), textureScreen.getHeight(), 0,
		textureScreen.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureScreen.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureScreen.freeImage(); // Liberamos memoria

	/*******************************************
	 * OpenAL init
	 *******************************************/
	alutInit(0, nullptr);
	alListenerfv(AL_POSITION, listenerPos);
	alListenerfv(AL_VELOCITY, listenerVel);
	alListenerfv(AL_ORIENTATION, listenerOri);
	alGetError(); // clear any error messages
	if (alGetError() != AL_NO_ERROR) {
		printf("- Error creating buffers !!\n");
		exit(1);
	}
	else {
		printf("init() - No errors yet.");
	}
	// Generate buffers, or else no sound will happen!
	alGenBuffers(NUM_BUFFERS, buffer);
	buffer[0] = alutCreateBufferFromFile("../sounds/fountain.wav");
	buffer[1] = alutCreateBufferFromFile("../sounds/fire.wav");
	buffer[2] = alutCreateBufferFromFile("../sounds/darth_vader.wav");
	int errorAlut = alutGetError();
	if (errorAlut != ALUT_ERROR_NO_ERROR){
		printf("- Error open files with alut %d !!\n", errorAlut);
		exit(2);
	}

	alGetError(); /* clear error */
	alGenSources(NUM_SOURCES, source);

	if (alGetError() != AL_NO_ERROR) {
		printf("- Error creating sources !!\n");
		exit(2);
	}
	else {
		printf("init - no errors after alGenSources\n");
	}
	alSourcef(source[0], AL_PITCH, 1.0f);
	alSourcef(source[0], AL_GAIN, 3.0f);
	alSourcefv(source[0], AL_POSITION, source0Pos);
	alSourcefv(source[0], AL_VELOCITY, source0Vel);
	alSourcei(source[0], AL_BUFFER, buffer[0]);
	alSourcei(source[0], AL_LOOPING, AL_TRUE);
	alSourcef(source[0], AL_MAX_DISTANCE, 2000);

	alSourcef(source[1], AL_PITCH, 1.0f);
	alSourcef(source[1], AL_GAIN, 0.5f);
	alSourcefv(source[1], AL_POSITION, source1Pos);
	alSourcefv(source[1], AL_VELOCITY, source1Vel);
	alSourcei(source[1], AL_BUFFER, buffer[1]);
	alSourcei(source[1], AL_LOOPING, AL_TRUE);
	alSourcef(source[1], AL_MAX_DISTANCE, 1000);

	alSourcef(source[2], AL_PITCH, 1.0f);
	alSourcef(source[2], AL_GAIN, 0.3f);
	alSourcefv(source[2], AL_POSITION, source2Pos);
	alSourcefv(source[2], AL_VELOCITY, source2Vel);
	alSourcei(source[2], AL_BUFFER, buffer[2]);
	alSourcei(source[2], AL_LOOPING, AL_TRUE);
	alSourcef(source[2], AL_MAX_DISTANCE, 2000);

	// Inicializar el frameBuffer para almacenar la profundidad del render desde el punto de vista de la luz.
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor []= {1.0f, 1.0f, 1.0f, 1.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void destroy() {
	glfwDestroyWindow(window);
	glfwTerminate();
	// --------- IMPORTANTE ----------
	// Eliminar los shader y buffers creados.

	// Shaders Delete
	shader.destroy();
	shaderMulLighting.destroy();
	shaderSkybox.destroy();
	shaderTerrain.destroy();
	shaderDepth.destroy();
	shaderViewDepth.destroy();

	// Basic objects Delete
	skyboxSphere.destroy();
	boxCollider.destroy();
	sphereCollider.destroy();
	rayModel.destroy();
	boxIntro.destroy();
	boxViewDepth.destroy();

	// Custom objects Delete
	laberinto.destroy();
	fantasma.destroy();
	guardia.destroy();

	modelLamp1.destroy();
	modelLamp2.destroy();
	modelLampPost2.destroy();
	mayowModelAnimate.destroy();

	// Terrains objects Delete
	terrain.destroy();

	// Textures Delete
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &textureTerrainBID);
	glDeleteTextures(1, &textureTerrainGID);
	glDeleteTextures(1, &textureTerrainRID);
	glDeleteTextures(1, &textureTerrainBlendMapID);
	glDeleteTextures(1, &textureInit1ID);
	glDeleteTextures(1, &textureInit2ID);
	glDeleteTextures(1, &textureScreenID);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &depthMapFBO);

	// Cube Maps Delete
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDeleteTextures(1, &skyboxTextureID);
}

void reshapeCallback(GLFWwindow *Window, int widthRes, int heightRes) {
	screenWidth = widthRes;
	screenHeight = heightRes;
	glViewport(0, 0, widthRes, heightRes);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
		int mode) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			exitApp = true;
			break;
		}
	}
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
	offsetX = xpos - lastMousePosX;
	offsetY = ypos - lastMousePosY;
	lastMousePosX = xpos;
	lastMousePosY = ypos;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset){
	distanceFromTarget -= yoffset;
	camera->setDistanceFromTarget(distanceFromTarget);
}

void mouseButtonCallback(GLFWwindow *window, int button, int state, int mod) {
	if (state == GLFW_PRESS) {
		switch (button) {
		case GLFW_MOUSE_BUTTON_RIGHT:
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_LEFT:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			break;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			std::cout << "lastMousePos.x:" << lastMousePosX << std::endl;
			std::cout << "lastMousePos.y:" << lastMousePosY << std::endl;
			break;
		}
	}
}

bool processInput(bool continueApplication) {
	if (exitApp || glfwWindowShouldClose(window) != 0) {
		return false;
	}

	if(!iniciaPartida){
		bool presionarEnter = glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
		if(textureActivaID == textureInit1ID && presionarEnter){
			iniciaPartida = true;
			textureActivaID = textureScreenID;
		}
		else if(!presionarOpcion && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
			presionarOpcion = true;
			if(textureActivaID == textureInit1ID)
				textureActivaID = textureInit2ID;
			else if(textureActivaID == textureInit2ID)
				textureActivaID = textureInit1ID;
		}
		else if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
			presionarOpcion = false;
	}

	if (glfwJoystickPresent(GLFW_JOYSTICK_1) == GL_TRUE) {
		std::cout << "Esta presente el joystick" << std::endl;
		int axesCount, buttonCount;
		const float * axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
		std::cout << "Número de ejes disponibles :=>" << axesCount << std::endl;
		std::cout << "Left Stick X axis: " << axes[0] << std::endl;
		std::cout << "Left Stick Y axis: " << axes[1] << std::endl;
		std::cout << "Left Trigger/L2: " << axes[3] << std::endl;
		std::cout << "Right Stick X axis: " << axes[2] << std::endl;
		std::cout << "Right Stick Y axis: " << axes[5] << std::endl;
		std::cout << "Right Trigger/R2: " << axes[4] << std::endl;

		if(fabs(axes[1]) > 0.2){
			modelMatrixMayow = glm::translate(modelMatrixMayow, glm::vec3(0, 0, -axes[1] * 0.1));
			animationMayowIndex = 0;
		}if(fabs(axes[0]) > 0.2){
			modelMatrixMayow = glm::rotate(modelMatrixMayow, glm::radians(-axes[0] * 1.5f), glm::vec3(0, 1, 0));
			animationMayowIndex = 0;
		}

		if(fabs(axes[2]) > 0.2){
			camera->mouseMoveCamera(axes[2], 0.0, deltaTime);
		}if(fabs(axes[5]) > 0.2){
			camera->mouseMoveCamera(0.0, axes[5], deltaTime);
		}

		const unsigned char * buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
		std::cout << "Número de botones disponibles :=>" << buttonCount << std::endl;
		if(buttons[0] == GLFW_PRESS)
			std::cout << "Se presiona x" << std::endl;

	}

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		camera->mouseMoveCamera(offsetX, 0.0, deltaTime);
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		camera->mouseMoveCamera(0.0, offsetY, deltaTime);

	offsetX = 0;
	offsetY = 0;

	// Seleccionar modelo
	if (enableCountSelected && glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS){
		enableCountSelected = false;
		modelSelected++;
		if(modelSelected > 4)
			modelSelected = 0;
		std::cout << "modelSelected:" << modelSelected << std::endl;
	}
	else if(glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE)
		enableCountSelected = true;

	// Controles de mayow
	if (modelSelected == 0 && glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
		modelMatrixMayow = glm::rotate(modelMatrixMayow, 0.02f, glm::vec3(0, 1, 0));
		animationMayowIndex = 0;
	} else if (modelSelected == 0 && glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
		modelMatrixMayow = glm::rotate(modelMatrixMayow, -0.02f, glm::vec3(0, 1, 0));
		animationMayowIndex = 0;
	}
	if (modelSelected == 0 && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
		modelMatrixMayow = glm::translate(modelMatrixMayow, glm::vec3(0.0, 0.0, 0.02));
		animationMayowIndex = 0;
	}
	else if (modelSelected == 0 && glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
		modelMatrixMayow = glm::translate(modelMatrixMayow, glm::vec3(0.0, 0.0, -0.02));
		animationMayowIndex = 0;
	}

	glfwPollEvents();
	return continueApplication;
}

void prepareScene(){

	terrain.setShader(&shaderTerrain);

	//Lamp models
	modelLamp1.setShader(&shaderMulLighting);
	modelLamp2.setShader(&shaderMulLighting);
	modelLampPost2.setShader(&shaderMulLighting);

	//Mayow
	mayowModelAnimate.setShader(&shaderMulLighting);

	// Laberinto
	laberinto.setShader(&shaderMulLighting);

	// Fantasma
	fantasma.setShader(&shaderMulLighting);

	// Guardia
	guardia.setShader(&shaderMulLighting);

}

void prepareDepthScene(){

	terrain.setShader(&shaderDepth);

	//Lamp models
	modelLamp1.setShader(&shaderDepth);
	modelLamp2.setShader(&shaderDepth);
	modelLampPost2.setShader(&shaderDepth);

	//Mayow
	mayowModelAnimate.setShader(&shaderDepth);

	// Laberinto
	laberinto.setShader(&shaderDepth);

	// Fantasma
	fantasma.setShader(&shaderDepth);

	// Guardia
	guardia.setShader(&shaderDepth);

}

// Dentro de esta funcion estaran todos los elementos que generarán sombras
void renderSolidScene(){
	/*******************************************
	 * Terrain Cesped
	 *******************************************/
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureTerrainRID);
	shaderTerrain.setInt("backgroundTexture", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureTerrainRID);
	shaderTerrain.setInt("rTexture", 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureTerrainGID);
	shaderTerrain.setInt("gTexture", 2);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBID);
	shaderTerrain.setInt("bTexture", 3);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, textureTerrainBlendMapID);
	shaderTerrain.setInt("blendMapTexture", 4);
	shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(80, 80)));
	terrain.setPosition(glm::vec3(100, 0, 100));
	terrain.render();
	shaderTerrain.setVectorFloat2("scaleUV", glm::value_ptr(glm::vec2(0, 0)));
	glBindTexture(GL_TEXTURE_2D, 0);

	/*******************************************
	 * Custom objects obj
	 *******************************************/

	// Forze to enable the unit texture to 0 always ----------------- IMPORTANT
	glActiveTexture(GL_TEXTURE0);

	// Render lamp
	for(int i = 0; i < lamp1Position.size(); i++){
		lamp1Position[i].y = terrain.getHeightTerrain(lamp1Position[i].x, lamp1Position[i].z);
		modelLamp1.setPosition(lamp1Position[i]);
		modelLamp1.setScale(glm::vec3(0.5));
		modelLamp1.setOrientation(glm::vec3(0, lamp1Orientation[i], 0));
		modelLamp1.render();
	}
	for(int i = 0; i < lamp2Position.size(); i++){
		lamp2Position[i].y = terrain.getHeightTerrain(lamp2Position[i].x, lamp2Position[i].z);
		modelLamp2.setPosition(lamp2Position[i]);
		modelLamp2.setScale(glm::vec3(0.5));
		modelLamp2.setOrientation(glm::vec3(0, lamp2Orientation[i], 0));
		modelLamp2.render();
		modelLampPost2.setPosition(lamp2Position[i]);
		modelLampPost2.setScale(glm::vec3(0.5));
		modelLampPost2.setOrientation(glm::vec3(0, lamp2Orientation[i], 0));
		modelLampPost2.render();
	}

	/*****************************************
	 * Objetos animados por huesos
	 * **************************************/
	glm::vec3 ejey = glm::normalize(terrain.getNormalTerrain(modelMatrixMayow[3][0], modelMatrixMayow[3][2]));
	glm::vec3 ejex = glm::vec3(modelMatrixMayow[0]);
	glm::vec3 ejez = glm::normalize(glm::cross(ejex, ejey));
	ejex = glm::normalize(glm::cross(ejey, ejez));
	modelMatrixMayow[0] = glm::vec4(ejex, 0.0);
	modelMatrixMayow[1] = glm::vec4(ejey, 0.0);
	modelMatrixMayow[2] = glm::vec4(ejez, 0.0);
	glm::mat4 modelMatrixMayowBody = glm::mat4(modelMatrixMayow);
	modelMatrixMayowBody = glm::scale(modelMatrixMayowBody, glm::vec3(0.021f));
	mayowModelAnimate.setAnimationIndex(animationMayowIndex);
	mayowModelAnimate.render(modelMatrixMayowBody);

	modelMatrixGuardia[3][1] = terrain.getHeightTerrain(modelMatrixGuardia[3][0], modelMatrixGuardia[3][2]);
	glm::mat4 modelMatrixGuardiaBody = glm::mat4(modelMatrixGuardia);
	guardia.render(modelMatrixGuardiaBody);

	// Interpolación lineal para mover el fantasma hacia la posición de Mayow
	float interpolationFactor = 0.001f; // Ajusta este valor para cambiar la velocidad de seguimiento
	glm::vec3 newPositionFantasma = glm::mix(glm::vec3(modelMatrixFantasma[3]), glm::vec3(modelMatrixMayow[3]), interpolationFactor);

	// Calcular la dirección hacia Mayow y ajustar la orientación del fantasma
	glm::vec3 directionFantasma = glm::normalize(glm::vec3(modelMatrixMayow[3]) - newPositionFantasma);
	glm::vec3 upFantasma = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 rightFantasma = glm::normalize(glm::cross(upFantasma, directionFantasma));
	upFantasma = glm::cross(directionFantasma, rightFantasma);

	glm::mat4 rotationMatrixFantasma = glm::mat4(1.0f);
	rotationMatrixFantasma[0] = glm::vec4(rightFantasma, 0.0f);
	rotationMatrixFantasma[1] = glm::vec4(upFantasma, 0.0f);
	rotationMatrixFantasma[2] = glm::vec4(directionFantasma, 0.0f);

	// Actualizar la posición y orientación del fantasma
	modelMatrixFantasma[3] = glm::vec4(newPositionFantasma, 1.0f);
	modelMatrixFantasma = glm::translate(glm::mat4(1.0f), newPositionFantasma) * rotationMatrixFantasma;


	modelMatrixFantasma[3][1] = terrain.getHeightTerrain(modelMatrixFantasma[3][0], modelMatrixFantasma[3][2]);
	glm::mat4 modelMatrixFantasmaBody = glm::mat4(modelMatrixFantasma);
	fantasma.render(modelMatrixFantasmaBody);

	laberinto.render(modelMatrixLaberinto);

	/*******************************************
	 * Skybox
	 *******************************************/
	GLint oldCullFaceMode;
	GLint oldDepthFuncMode;
	// deshabilita el modo del recorte de caras ocultas para ver las esfera desde adentro
	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFaceMode);
	glGetIntegerv(GL_DEPTH_FUNC, &oldDepthFuncMode);
	shaderSkybox.setFloat("skybox", 0);
	glCullFace(GL_FRONT);
	glDepthFunc(GL_LEQUAL);
	glActiveTexture(GL_TEXTURE0);
	skyboxSphere.render();
	glCullFace(oldCullFaceMode);
	glDepthFunc(oldDepthFuncMode);
}
// Aqui van los objetos transparentes.
void renderAlphaScene(bool render = true){
	/**********Render de transparencias***************/
	/**********
	 * Update the position with alpha objects
	 */
	// Update the aircraft
	//blendingUnsorted.find("aircraft")->second = glm::vec3(modelMatrixAircraft[3]);

	/**********
	 * Sorter with alpha objects
	 */
	std::map<float, std::pair<std::string, glm::vec3>> blendingSorted;
	std::map<std::string, glm::vec3>::iterator itblend;

	/**********
	 * Render de las transparencias
	 */
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	if(render){
		/************Render de imagen de frente**************/
		shaderTexture.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
		shaderTexture.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureActivaID);
		shaderTexture.setInt("outTexture", 0);
		glEnable(GL_BLEND);
		boxIntro.render();
		glDisable(GL_BLEND);

		modelText->render("Texto en OpenGL", -1, 0);
	}
}

void renderScene(){
	renderSolidScene();
	renderAlphaScene(false);
}

void applicationLoop() {
	bool psi = true;

	glm::vec3 axis;
	glm::vec3 target;
	float angleTarget;

	int state = 0;
	float advanceCount = 0.0;
	float rotCount = 0.0;
	int numberAdvance = 0;
	int maxAdvance = 0.0;

	modelMatrixMayow = glm::translate(modelMatrixMayow, glm::vec3(34.677f, 0.05f, 37.0987f));
	modelMatrixMayow = glm::rotate(modelMatrixMayow, glm::radians(-90.0f), glm::vec3(0, 1, 0));

	lastTime = TimeManager::Instance().GetTime();

	textureActivaID = textureInit1ID;

	glm::vec3 lightPos = glm::vec3(10.0, 10.0, -10.0);

	shadowBox = new ShadowBox(-lightPos,camera.get(),15.0f,0.1,45.0f);

	while (psi) {
		currTime = TimeManager::Instance().GetTime();
		if(currTime - lastTime < 0.016666667){
			glfwPollEvents();
			continue;
		}
		lastTime = currTime;
		TimeManager::Instance().CalculateFrameRate(true);
		deltaTime = TimeManager::Instance().DeltaTime;
		psi = processInput(true);

		std::map<std::string, bool> collisionDetection;
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 projection = glm::perspective(glm::radians(45.0f),
				(float) screenWidth / (float) screenHeight, 0.01f, 100.0f);

		
		axis = glm::axis(glm::quat_cast(modelMatrixMayow));
		angleTarget = glm::angle(glm::quat_cast(modelMatrixMayow));
		target = modelMatrixMayow[3];
		
		if(std::isnan(angleTarget))
			angleTarget = 0.0;
		if(axis.y < 0)
			angleTarget = -angleTarget;

		camera->setCameraTarget(target);
		camera->setAngleTarget(angleTarget);
		camera->updateCamera();
		glm::mat4 view = camera->getViewMatrix();

		shadowBox->update(screenWidth,screenHeight);
		glm::vec3 centerBox = shadowBox->getCenter();

		// Projection Light Shadow mapping
		glm::mat4 lightProjection = glm::mat4(1.0f), lightView = glm::mat4(1.0f);
		glm::mat4 lightSpaceMatrix;
		float near_plane = 0.1f, far_plane = 20.0f;
		//lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
		//lightView = glm::lookAt(lightPos, glm::vec3(0.0), glm::vec3(0,1,0));
		lightProjection[0][0] = 2.0 / shadowBox->getWidth();
		lightProjection[1][1] = 2.0 / shadowBox->getHeight();
		lightProjection[2][2] = -2.0 / shadowBox->getLength();
		lightProjection[3][3] = 1.0;
		lightView = glm::lookAt(centerBox,centerBox + glm::normalize(-lightPos), glm::vec3(0,1,0));
		lightSpaceMatrix = lightProjection * lightView;
		shaderDepth.setMatrix4("lightSpaceMatrix", 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

		// Settea la matriz de vista y projection al shader con solo color
		shader.setMatrix4("projection", 1, false, glm::value_ptr(projection));
		shader.setMatrix4("view", 1, false, glm::value_ptr(view));

		// Settea la matriz de vista y projection al shader con skybox
		shaderSkybox.setMatrix4("projection", 1, false,
				glm::value_ptr(projection));
		shaderSkybox.setMatrix4("view", 1, false,
				glm::value_ptr(glm::mat4(glm::mat3(view))));
		// Settea la matriz de vista y projection al shader con multiples luces
		shaderMulLighting.setMatrix4("projection", 1, false,
					glm::value_ptr(projection));
		shaderMulLighting.setMatrix4("view", 1, false,
				glm::value_ptr(view));
		// Settea la matriz de vista y projection al shader con multiples luces
		shaderTerrain.setMatrix4("projection", 1, false,
				glm::value_ptr(projection));
		shaderTerrain.setMatrix4("view", 1, false,
				glm::value_ptr(view));

		/*******************************************
		 * Propiedades de neblina
		 *******************************************/
		shaderMulLighting.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
		shaderTerrain.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
		shaderSkybox.setVectorFloat3("fogColor", glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));

		/*******************************************
		 * Propiedades Luz direccional
		 *******************************************/
		shaderMulLighting.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderMulLighting.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.4, 0.4, 0.4)));
		shaderMulLighting.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.7071, -0.7071, -0.7071)));

		shaderTerrain.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderTerrain.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
		shaderTerrain.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderTerrain.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.4, 0.4, 0.4)));
		shaderTerrain.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.7071, -0.7071, -0.7071)));
		
		/*******************************************
		 * Propiedades PointLights
		 *******************************************/
		shaderMulLighting.setInt("pointLightCount", lamp1Position.size() + lamp2Position.size());
		shaderTerrain.setInt("pointLightCount", lamp1Position.size() + lamp2Position.size());
		for(int i = 0; i < lamp1Position.size(); i++){
			glm::mat4 matrixAdjustLamp = glm::mat4(1.0);
			matrixAdjustLamp = glm::translate(matrixAdjustLamp, lamp1Position[i]);
			matrixAdjustLamp = glm::rotate(matrixAdjustLamp, glm::radians(lamp1Orientation[i]), glm::vec3(0, 1, 0));
			matrixAdjustLamp = glm::scale(matrixAdjustLamp, glm::vec3(0.5));
			matrixAdjustLamp = glm::translate(matrixAdjustLamp, glm::vec3(0.0, 10.35, 0));
			glm::vec3 lampPosition = glm::vec3(matrixAdjustLamp[3]);
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.4, 0.32, 0.02)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].position", glm::value_ptr(lampPosition));
			shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0);
			shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09);
			shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.02);
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.4, 0.32, 0.02)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].position", glm::value_ptr(lampPosition));
			shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0);
			shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09);
			shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.02);
		}
		for(int i = 0; i < lamp2Position.size(); i++){
			glm::mat4 matrixAdjustLamp = glm::mat4(1.0);
			matrixAdjustLamp = glm::translate(matrixAdjustLamp, lamp2Position[i]);
			matrixAdjustLamp = glm::rotate(matrixAdjustLamp, glm::radians(lamp2Orientation[i]), glm::vec3(0, 1, 0));
			matrixAdjustLamp = glm::scale(matrixAdjustLamp, glm::vec3(1.0));
			matrixAdjustLamp = glm::translate(matrixAdjustLamp, glm::vec3(0.75, 5.0, 0));
			glm::vec3 lampPosition = glm::vec3(matrixAdjustLamp[3]);
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].light.ambient", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.4, 0.32, 0.02)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].light.specular", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].position", glm::value_ptr(lampPosition));
			shaderMulLighting.setFloat("pointLights[" + std::to_string(lamp1Position.size() + i) + "].constant", 1.0);
			shaderMulLighting.setFloat("pointLights[" + std::to_string(lamp1Position.size() + i) + "].linear", 0.09);
			shaderMulLighting.setFloat("pointLights[" + std::to_string(lamp1Position.size() + i) + "].quadratic", 0.02);
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].light.ambient", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.4, 0.32, 0.02)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].light.specular", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(lamp1Position.size() + i) + "].position", glm::value_ptr(lampPosition));
			shaderTerrain.setFloat("pointLights[" + std::to_string(lamp1Position.size() + i) + "].constant", 1.0);
			shaderTerrain.setFloat("pointLights[" + std::to_string(lamp1Position.size() + i) + "].linear", 0.09);
			shaderTerrain.setFloat("pointLights[" + std::to_string(lamp1Position.size() + i) + "].quadratic", 0.02);
		}

		//Para sombras
		shaderTerrain.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));
		shaderMulLighting.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));

		/************Render de imagen de frente**************/
		if(!iniciaPartida){
			shaderTexture.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
			shaderTexture.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, textureActivaID);
			shaderTexture.setInt("outTexture", 0);
			boxIntro.render();
			glfwSwapBuffers(window);
			continue;
		}

		// Render del buffer de profundidad desde el punto de vista de la luz
		glClearColor(0.1f,0.1f,0.1f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		prepareDepthScene();
		renderScene();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Para debug
		glViewport(0,0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Render solo para debug
		shaderViewDepth.setMatrix4("projection", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		shaderViewDepth.setMatrix4("view", 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		shaderViewDepth.setFloat("near_plane", near_plane);
		shaderViewDepth.setFloat("far_plane", far_plane);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		boxViewDepth.setScale(glm::vec3(2.0, 2.0, 1.0));
		boxViewDepth.render();

		// Render normal de los objetos
		glViewport(0,0,screenWidth,screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		prepareScene();
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		shaderMulLighting.setInt("shadowMap", 10);
		shaderTerrain.setInt("shadowMap", 10);
		renderSolidScene();

		/*******************************************
		 * Creacion de colliders
		 * IMPORTANT do this before interpolations
		 *******************************************/

		// Lamps1 colliders
		for (int i = 0; i < lamp1Position.size(); i++){
			AbstractModel::OBB lampCollider;
			glm::mat4 modelMatrixColliderLamp = glm::mat4(1.0);
			modelMatrixColliderLamp = glm::translate(modelMatrixColliderLamp, lamp1Position[i]);
			modelMatrixColliderLamp = glm::rotate(modelMatrixColliderLamp, glm::radians(lamp1Orientation[i]),
					glm::vec3(0, 1, 0));
			addOrUpdateColliders(collidersOBB, "lamp1-" + std::to_string(i), lampCollider, modelMatrixColliderLamp);
			// Set the orientation of collider before doing the scale
			lampCollider.u = glm::quat_cast(modelMatrixColliderLamp);
			modelMatrixColliderLamp = glm::scale(modelMatrixColliderLamp, glm::vec3(0.5, 0.5, 0.5));
			modelMatrixColliderLamp = glm::translate(modelMatrixColliderLamp, modelLamp1.getObb().c);
			lampCollider.c = glm::vec3(modelMatrixColliderLamp[3]);
			lampCollider.e = modelLamp1.getObb().e * glm::vec3(0.5, 0.5, 0.5);
			std::get<0>(collidersOBB.find("lamp1-" + std::to_string(i))->second) = lampCollider;
		}

		// Lamps2 colliders
		for (int i = 0; i < lamp2Position.size(); i++){
			AbstractModel::OBB lampCollider;
			glm::mat4 modelMatrixColliderLamp = glm::mat4(1.0);
			modelMatrixColliderLamp = glm::translate(modelMatrixColliderLamp, lamp2Position[i]);
			modelMatrixColliderLamp = glm::rotate(modelMatrixColliderLamp, glm::radians(lamp2Orientation[i]),
					glm::vec3(0, 1, 0));
			addOrUpdateColliders(collidersOBB, "lamp2-" + std::to_string(i), lampCollider, modelMatrixColliderLamp);
			// Set the orientation of collider before doing the scale
			lampCollider.u = glm::quat_cast(modelMatrixColliderLamp);
			modelMatrixColliderLamp = glm::scale(modelMatrixColliderLamp, glm::vec3(1.0, 1.0, 1.0));
			modelMatrixColliderLamp = glm::translate(modelMatrixColliderLamp, modelLampPost2.getObb().c);
			lampCollider.c = glm::vec3(modelMatrixColliderLamp[3]);
			lampCollider.e = modelLampPost2.getObb().e * glm::vec3(1.0, 1.0, 1.0);
			std::get<0>(collidersOBB.find("lamp2-" + std::to_string(i))->second) = lampCollider;
		}

		// Define an estructura para mantener la información de cada collider
		struct ColliderData {
			std::string name;
			glm::vec3 translation;
			glm::vec3 scale;
			glm::vec3 offset;
		};

		// Colliders de laberinto
		std::vector<AbstractModel::OBB> laberintoColliders;
		std::vector<ColliderData> labCollidersData = {
			// Agrega aquí los datos de cada collider
			{"0", glm::vec3(-3.6324f + 0.1f, 0.0f, 38.1058f - 0.473f), glm::vec3(0.907, 1.0, 0.029), laberinto.getObb().c},
			{"1", glm::vec3(-38.1301f + 0.368f, 0.0f, -0.72f + 0.0f ), glm::vec3(0.0216, 1.0, 0.965), laberinto.getObb().c},
			{"2", glm::vec3(37.3468f + 0.368f, 0.0f, -2.80288f - 0.1f ), glm::vec3(0.023, 1.0, 0.93), laberinto.getObb().c},
			{"3", glm::vec3(2.49465f + 0.05f, 0.0f, -37.3267f - 0.443f), glm::vec3(0.897, 1.0, 0.025), laberinto.getObb().c},
			{"4", glm::vec3(2.49465f + 0.05f, 0.0f, -37.3267f - 0.443f), glm::vec3(0.897, 1.0, 0.025), laberinto.getObb().c},
			{"5", glm::vec3(-17.6765f + 0.3f, 0.0f, 32.2693f - 0.453f), glm::vec3(0.253, 1.0, 0.0265), laberinto.getObb().c},
			{"6", glm::vec3(-3.50801f + 0.35f, 0.0f, 32.2874f - 0.453f), glm::vec3(0.026, 1.0, 0.1205), laberinto.getObb().c},
			{"7", glm::vec3(-3.85522f + 0.31f, 0.0f, 26.5926f - 0.453f), glm::vec3(0.1616, 1.0, 0.0276), laberinto.getObb().c}, 
			{"8", glm::vec3(-9.05981f + 0.35f, 0.0f, 29.4483f - 0.453f), glm::vec3(0.028, 1.0, 0.0465), laberinto.getObb().c},
			{"9", glm::vec3(-29.4218f + 0.35f, 0.0f, 26.5666f - 0.453f), glm::vec3(0.1016, 1.0, 0.026), laberinto.getObb().c},
			{"10", glm::vec3(-26.526f + 0.35f, 0.0f, 29.3983f - 0.453f), glm::vec3(0.026, 1.0, 0.0475), laberinto.getObb().c},
			{"11", glm::vec3(-32.3587f + 0.35f, 0.0f, 30.0127f - 0.453f), glm::vec3(0.026, 1.0, 0.065), laberinto.getObb().c},
			{"12", glm::vec3(-31.4797f + 0.35f, 0.0f, 14.8692f - 0.453f), glm::vec3(0.149, 1.0, 0.0237), laberinto.getObb().c},
			{"13", glm::vec3(-20.6567f + 0.35f, 0.0f, 9.08339f - 0.453f), glm::vec3(0.169, 1.0, 0.019), laberinto.getObb().c},
			{"14", glm::vec3(-12.3017f + 0.35f, 0.0f, 14.8474f - 0.453f), glm::vec3(0.0875, 1.0, 0.021), laberinto.getObb().c},
			{"15", glm::vec3(-26.4918f + 0.35f, 0.0f, 11.8738f - 0.453f), glm::vec3(0.019, 1.0, 0.054), laberinto.getObb().c},
			{"16", glm::vec3(-14.9306f + 0.35f, 0.0f, 11.9138f - 0.453f), glm::vec3(0.02, 1.0, 0.055), laberinto.getObb().c},
			{"17", glm::vec3(-31.4897f + 0.35f, 0.0f, -31.4459f - 0.453f), glm::vec3(0.15, 1.0, 0.021), laberinto.getObb().c},
			{"18", glm::vec3(-26.5345f + 0.35f, 0.0f, -25.2295f - 0.453f), glm::vec3(0.021, 1.0, 0.141), laberinto.getObb().c},
			{"19", glm::vec3(-22.7926f + 0.35f, 0.0f, -25.6051f - 0.453f), glm::vec3(0.077, 1.0, 0.0235), laberinto.getObb().c},
			{"20", glm::vec3(-20.7658f + 0.35f, 0.0f, -17.1197f - 0.453f), glm::vec3(0.0243, 1.0, 0.198), laberinto.getObb().c},
			{"21", glm::vec3(-20.7328f + 0.35f, 0.0f, -8.28627f - 0.453f), glm::vec3(0.175, 1.0, 0.0285), laberinto.getObb().c},
			{"22", glm::vec3(-26.624f + 0.35f, 0.0f, -4.58668f - 0.453f), glm::vec3(0.0235, 1.0, 0.068), laberinto.getObb().c},
			{"23", glm::vec3(-14.9286f + 0.35f, 0.0f, -12.1999f - 0.453f), glm::vec3(0.0235, 1.0, 0.073), laberinto.getObb().c},
			{"24", glm::vec3(-11.4195f + 0.35f, 0.0f, -14.114f - 0.453f), glm::vec3(0.067, 1.0, 0.023), laberinto.getObb().c},
			{"25", glm::vec3(-14.0023f + 0.35f, 0.0f, -19.8068f - 0.453f), glm::vec3(0.151, 1.0, 0.026), laberinto.getObb().c},
			{"26", glm::vec3(-9.01007f + 0.35f, 0.0f, -23.7647f - 0.453f), glm::vec3(0.021, 1.0, 0.077), laberinto.getObb().c},
			{"27", glm::vec3(-6.11044f + 0.35f, 0.0f, -25.7411f - 0.453f), glm::vec3(0.054, 1.0, 0.0255), laberinto.getObb().c},
			{"28", glm::vec3(-3.05288f + 0.35f, 0.0f, -13.9236f - 0.253f), glm::vec3(0.0259, 1.0, 0.46), laberinto.getObb().c},
			{"29", glm::vec3(0.329842f + 0.35f, 0.0f, -8.46291f - 0.453f), glm::vec3(0.064, 1.0, 0.027), laberinto.getObb().c},
			{"30", glm::vec3(3.61074f + 0.35f, 0.0f, 3.18889f - 0.453f), glm::vec3(0.15, 1.0, 0.0226), laberinto.getObb().c},
			{"31", glm::vec3(8.37367f + 0.35f, 0.0f, 8.93977f - 0.453f), glm::vec3(0.026, 1.0, 0.13), laberinto.getObb().c},
			{"32", glm::vec3(8.58592f + 0.35f, 0.0f, 14.9383f - 0.453f), glm::vec3(0.164, 1.0, 0.024), laberinto.getObb().c},
			{"33", glm::vec3(13.9548f + 0.35f, 0.0f, 21.1937f - 0.453f), glm::vec3(0.024, 1.0, 0.139), laberinto.getObb().c},
			{"34", glm::vec3(20.3491f + 0.35f, 0.0f, 20.7249f - 0.453f), glm::vec3(0.141, 1.0, 0.0285), laberinto.getObb().c},
			{"35", glm::vec3(15.2893f + 0.35f, 0.0f, 8.98606f - 0.453f), glm::vec3(0.1525, 1.0, 0.0295), laberinto.getObb().c},
			{"36", glm::vec3(19.8336f + 0.35f, 0.0f, 13.0158f - 0.453f), glm::vec3(0.0336, 1.0, 0.076), laberinto.getObb().c},
			{"37", glm::vec3(25.6743f + 0.35f, 0.0f, 14.7458f - 0.453f), glm::vec3(0.117, 1.0, 0.032), laberinto.getObb().c},
			{"38", glm::vec3(31.3753f + 0.35f, 0.0f, 19.584f - 0.353f), glm::vec3(0.032, 1.0, 0.29), laberinto.getObb().c},
			{"39", glm::vec3(17.24f + 0.35f, 0.0f, 32.3325f - 0.453f), glm::vec3(0.4005, 1.0, 0.0381), laberinto.getObb().c},
			{"40", glm::vec3(19.7336f + 0.35f, 0.0f, 27.8722f - 0.453f), glm::vec3(0.0436, 1.0, 0.078), laberinto.getObb().c},
			{"41", glm::vec3(23.6194f + 0.33f, 0.0f, 26.4013f - 0.453f), glm::vec3(0.0572, 1.0, 0.0393), laberinto.getObb().c},
			{"42", glm::vec3(8.13572f + 0.35f, 0.0f, 26.2581f - 0.453f), glm::vec3(0.0343, 1.0, 0.118), laberinto.getObb().c},
			{"43", glm::vec3(-3.3065f + 0.25f, 0.0f, 20.6666f - 0.453f), glm::vec3(0.333, 1.0, 0.0289), laberinto.getObb().c},
			{"44", glm::vec3(-15.107f + 0.35f, 0.0f, 24.6693f - 0.453f), glm::vec3(0.03, 1.0, 0.0765), laberinto.getObb().c},
			{"45", glm::vec3(-19.116f + 0.35f, 0.0f, 26.6386f - 0.453f), glm::vec3(0.0751, 1.0, 0.0261), laberinto.getObb().c},
			{"46", glm::vec3(-20.8518f + 0.35f, 0.0f, 19.9416f - 0.453f), glm::vec3(0.0302, 1.0, 0.1457), laberinto.getObb().c},
			{"47", glm::vec3(-27.3592f + 0.35f, 0.0f, 20.6466f - 0.453f), glm::vec3(0.141, 1.0, 0.0297), laberinto.getObb().c},
			{"48", glm::vec3(-3.34545f + 0.35f, 0.0f, 14.7634f - 0.453f), glm::vec3(0.0275, 1.0, 0.124), laberinto.getObb().c},
			{"49", glm::vec3(-3.72143f + 0.33f, 0.0f, 8.90698f - 0.453f), glm::vec3(0.166, 1.0, 0.0292), laberinto.getObb().c},
			{"50", glm::vec3(-9.14382f + 0.45f, 0.0f, 4.9802f - 0.453f), glm::vec3(0.0267, 1.0, 0.072), laberinto.getObb().c},
			{"51", glm::vec3(-20.7125f + 0.35f, 0.0f, 3.22433f - 0.453f), glm::vec3(0.275, 1.0, 0.026), laberinto.getObb().c},
			{"52", glm::vec3(-20.6354f + 0.35f, 0.0f, -0.750006f - 0.453f), glm::vec3(0.0278, 1.0, 0.0763), laberinto.getObb().c},
			{"53", glm::vec3(-13.8682f + 0.40f, 0.0f, -2.5535f - 0.453f), glm::vec3(0.147, 1.0, 0.0292), laberinto.getObb().c},
			{"54", glm::vec3(-9.20505f + 0.35f, 0.0f, -6.13236f - 0.453f), glm::vec3(0.0252, 1.0, 0.0635), laberinto.getObb().c},
			{"55", glm::vec3(-32.3147f + 0.35f, 0.0f, -8.40289f - 0.253f), glm::vec3(0.0241, 1.0, 0.456), laberinto.getObb().c},
			{"56", glm::vec3(-28.8165f + 0.35f, 0.0f, -14.1171f - 0.453f), glm::vec3(0.0665, 1.0, 0.0295), laberinto.getObb().c},
			{"57", glm::vec3(-20.8323f + 0.35f, 0.0f, -33.4034f - 0.453f), glm::vec3(0.0246, 1.0, 0.077), laberinto.getObb().c},
			{"58", glm::vec3(-14.4759f + 0.35f, 0.0f, -31.4079f - 0.403f), glm::vec3(0.139, 1.0, 0.026), laberinto.getObb().c},
			{"59", glm::vec3(-14.8705f + 0.35f, 0.0f, -28.0781f - 0.453f), glm::vec3(0.024, 1.0, 0.061), laberinto.getObb().c},
			{"60", glm::vec3(2.57207f + 0.35f, 0.0f, -24.9437f - 0.353f), glm::vec3(0.0215, 1.0, 0.3), laberinto.getObb().c},
			{"61", glm::vec3(6.3539f + 0.35f, 0.0f, -14.2633f - 0.453f), glm::vec3(0.077, 1.0, 0.027), laberinto.getObb().c},
			{"62", glm::vec3(8.30386f + 0.35f, 0.0f, -8.45836f - 0.453f), glm::vec3(0.0259, 1.0, 0.126), laberinto.getObb().c},
			{"63", glm::vec3(8.57676f + 0.3f, 0.0f, -2.55884f - 0.453f), glm::vec3(0.1631, 1.0, 0.026), laberinto.getObb().c},
			{"64", glm::vec3(14.1175f + 0.35f, 0.0f, 1.24933f - 0.453f), glm::vec3(0.0185, 1.0, 0.073), laberinto.getObb().c},
			{"65", glm::vec3(23.2535f + 0.3f, 0.0f, 3.28111f - 0.453f), glm::vec3(0.217, 1.0, 0.0213), laberinto.getObb().c},
			{"66", glm::vec3(25.5824f + 0.4f, 0.0f, 6.65904f - 0.453f), glm::vec3(0.017, 1.0, 0.067), laberinto.getObb().c},
			{"67", glm::vec3(19.8817f + 0.35f, 0.0f, -3.44269f - 0.403f), glm::vec3(0.021, 1.0, 0.153), laberinto.getObb().c},
			{"68", glm::vec3(23.6537f + 0.35f, 0.0f, -8.43151f - 0.453f), glm::vec3(0.0762, 1.0, 0.0225), laberinto.getObb().c},
			{"69", glm::vec3(25.7076f + 0.35f, 0.0f, -12.3049f - 0.453f), glm::vec3(0.0226, 1.0, 0.077), laberinto.getObb().c},
			{"70", glm::vec3(19.9355f + 0.35f, 0.0f, -14.3352f - 0.453f), glm::vec3(0.128, 1.0, 0.024), laberinto.getObb().c},
			{"71", glm::vec3(14.1791f + 0.35f, 0.0f, -14.4008f - 0.453f), glm::vec3(0.0205, 1.0, 0.1658), laberinto.getObb().c},
			{"72", glm::vec3(10.5119f + 0.35f, 0.0f, -19.8979f - 0.453f), glm::vec3(0.0753, 1.0, 0.022), laberinto.getObb().c},
			{"73", glm::vec3(8.29417f + 0.4f, 0.0f, -23.3479f - 0.453f), glm::vec3(0.0185, 1.0, 0.067), laberinto.getObb().c},
			{"74", glm::vec3(31.0499f + 0.35f, 0.0f, -2.50399f - 0.453f), glm::vec3(0.143, 1.0, 0.029), laberinto.getObb().c},
			{"75", glm::vec3(31.4071f + 0.35f, 0.0f, -12.3674f - 0.353f), glm::vec3(0.022, 1.0, 0.2239), laberinto.getObb().c},
			{"76", glm::vec3(24.8801f + 0.35f, 0.0f, -19.9493f - 0.453f), glm::vec3(0.15, 1.0, 0.026), laberinto.getObb().c},
			{"77", glm::vec3(19.9578f + 0.35f, 0.0f, -23.8032f - 0.453f), glm::vec3(0.0236, 1.0, 0.0745), laberinto.getObb().c},
			{"78", glm::vec3(16.232f + 0.35f, 0.0f, -25.8356f - 0.453f), glm::vec3(0.075, 1.0, 0.0216), laberinto.getObb().c},
			{"79", glm::vec3(14.1309f + 0.35f, 0.0f, -28.7281f - 0.453f), glm::vec3(0.021, 1.0, 0.053), laberinto.getObb().c},
			{"80", glm::vec3(17.5303f + 0.3f, 0.0f, -31.7807f - 0.453f), glm::vec3(0.241, 1.0, 0.0262), laberinto.getObb().c},
			{"81", glm::vec3(25.7199f + 0.35f, 0.0f, -27.7341f - 0.453f), glm::vec3(0.027, 1.0, 0.079), laberinto.getObb().c},
			{"82", glm::vec3(29.1983f + 0.35f, 0.0f, -25.7858f - 0.453f), glm::vec3(0.063, 1.0, 0.0295), laberinto.getObb().c},
			{"83", glm::vec3(31.6253f + 0.35f, 0.0f, -33.8753f - 0.453f), glm::vec3(0.0236, 1.0, 0.0647), laberinto.getObb().c},

		};

		for (const auto& data : labCollidersData) {
			AbstractModel::OBB laberintoCollider;
			glm::mat4 modelmatrixColliderLaberinto = glm::mat4(modelMatrixLaberinto);

			// Set the orientation of collider before doing the scale
			laberintoCollider.u = glm::quat_cast(modelmatrixColliderLaberinto);
			modelmatrixColliderLaberinto = glm::translate(modelmatrixColliderLaberinto, data.translation + data.offset);
			modelmatrixColliderLaberinto = glm::scale(modelmatrixColliderLaberinto, data.scale);
			
			laberintoCollider.c = glm::vec3(modelmatrixColliderLaberinto[3]);
			laberintoCollider.e = laberinto.getObb().e * data.scale;
			
			addOrUpdateColliders(collidersOBB, " laberinto " + data.name, laberintoCollider, modelMatrixLaberinto);
			laberintoColliders.push_back(laberintoCollider);
		}


		// Collider de mayow
		AbstractModel::OBB mayowCollider;
		glm::mat4 modelmatrixColliderMayow = glm::mat4(modelMatrixMayow);
		modelmatrixColliderMayow = glm::rotate(modelmatrixColliderMayow,
				glm::radians(-90.0f), glm::vec3(1, 0, 0));
		// Set the orientation of collider before doing the scale
		mayowCollider.u = glm::quat_cast(modelmatrixColliderMayow);
		modelmatrixColliderMayow = glm::scale(modelmatrixColliderMayow, glm::vec3(0.021, 0.021, 0.021));
		modelmatrixColliderMayow = glm::translate(modelmatrixColliderMayow,
				glm::vec3(mayowModelAnimate.getObb().c.x,
						mayowModelAnimate.getObb().c.y,
						mayowModelAnimate.getObb().c.z));
		mayowCollider.e = mayowModelAnimate.getObb().e * glm::vec3(0.021, 0.021, 0.021) * glm::vec3(0.787401574, 0.787401574, 0.787401574);
		mayowCollider.c = glm::vec3(modelmatrixColliderMayow[3]);
		addOrUpdateColliders(collidersOBB, "mayow", mayowCollider, modelMatrixMayow);

		// Collider de fantasma
		AbstractModel::OBB fantasmaCollider;
		glm::mat4 modelmatrixColliderFantasma = glm::mat4(modelMatrixFantasma);
		// Set the orientation of collider before doing the scale
		fantasmaCollider.u = glm::quat_cast(modelmatrixColliderFantasma);
		modelmatrixColliderFantasma = glm::scale(modelmatrixColliderFantasma, glm::vec3(1.0, 1.0, 1.0));
		modelmatrixColliderFantasma = glm::translate(modelmatrixColliderFantasma,
				glm::vec3(fantasma.getObb().c.x,
						fantasma.getObb().c.y,
						fantasma.getObb().c.z));
		fantasmaCollider.e = fantasma.getObb().e * glm::vec3(1.0, 1.0, 1.0);
		fantasmaCollider.c = glm::vec3(modelmatrixColliderFantasma[3]);
		addOrUpdateColliders(collidersOBB, "fantasma", fantasmaCollider, modelMatrixFantasma);

		/*******************************************
		 * Render de colliders
		 *******************************************/
		
		for (std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4> >::iterator it =
				collidersOBB.begin(); it != collidersOBB.end(); it++) {
			glm::mat4 matrixCollider = glm::mat4(1.0);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(it->second).c);
			matrixCollider = matrixCollider * glm::mat4(std::get<0>(it->second).u);
			matrixCollider = glm::scale(matrixCollider, std::get<0>(it->second).e * 2.0f);
			boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			boxCollider.enableWireMode();
			boxCollider.render(matrixCollider);
		}

		for (std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4> >::iterator it =
				collidersSBB.begin(); it != collidersSBB.end(); it++) {
			glm::mat4 matrixCollider = glm::mat4(1.0);
			matrixCollider = glm::translate(matrixCollider, std::get<0>(it->second).c);
			matrixCollider = glm::scale(matrixCollider, glm::vec3(std::get<0>(it->second).ratio * 2.0f));
			sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
			sphereCollider.enableWireMode();
			sphereCollider.render(matrixCollider);
		}


		/*********************Prueba de colisiones****************************/
		for (std::map<std::string,
			std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it =
			collidersSBB.begin(); it != collidersSBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string,
				std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator jt =
				collidersSBB.begin(); jt != collidersSBB.end(); jt++) {
				if (it != jt && testSphereSphereIntersection(
					std::get<0>(it->second), std::get<0>(jt->second))) {
					//std::cout << "Hay collision entre " << it->first <<
					//	" y el modelo " << jt->first << std::endl;
					isCollision = true;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}

		for (std::map<std::string,
			std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator it =
			collidersOBB.begin(); it != collidersOBB.end(); it++) {
			bool isColision = false;
			for (std::map<std::string,
				std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator jt =
				collidersOBB.begin(); jt != collidersOBB.end(); jt++) {
				if (it != jt && 
					testOBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
					// Aqui se va a evaluar si el guardia y/o el fantasma chocaron con el jugador.
					if ((it->first == "fantasma" && jt->first == "mayow") || (jt->first == "fantasma" && it->first == "mayow")) 
					{
						std::cout << "El fantasma ha colisionado con mayow. Cerrando el juego..." << std::endl;
						return;
					}
					//std::cout << "Hay colision entre " << it->first << " y el modelo" <<
						//jt->first << std::endl;
					isColision = true;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isColision);
		}

		for (std::map<std::string,
			std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::iterator it =
			collidersSBB.begin(); it != collidersSBB.end(); it++) {
			bool isCollision = false;
			for (std::map<std::string,
				std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::iterator jt =
				collidersOBB.begin(); jt != collidersOBB.end(); jt++) {
				if (testSphereOBox(std::get<0>(it->second), std::get<0>(jt->second))) {
					//std::cout << "Hay colision del " << it->first << " y el modelo" <<
					//	jt->first << std::endl;
					isCollision = true;
					addOrUpdateCollisionDetection(collisionDetection, jt->first, true);
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, it->first, isCollision);
		}

		std::map<std::string, bool>::iterator itCollision;
		for (itCollision = collisionDetection.begin(); 
			itCollision != collisionDetection.end(); itCollision++) {
			std::map<std::string, std::tuple<AbstractModel::SBB, 
				glm::mat4, glm::mat4>>::iterator sbbBuscado = 
				collidersSBB.find(itCollision->first);
			std::map<std::string, std::tuple<AbstractModel::OBB,
				glm::mat4, glm::mat4>>::iterator obbBuscado =
				collidersOBB.find(itCollision->first);
			if (sbbBuscado != collidersSBB.end()) {
				if (!itCollision->second) 
					addOrUpdateColliders(collidersSBB, itCollision->first);
			}
			if (obbBuscado != collidersOBB.end()) {
				if (!itCollision->second) 
					addOrUpdateColliders(collidersOBB, itCollision->first);
				else {
					if (itCollision->first.compare("mayow") == 0)
						modelMatrixMayow = std::get<1>(obbBuscado->second);
				}
			}
		}

		glm::mat4 modelMatrixRayMay = glm::mat4(modelMatrixMayow);
		modelMatrixRayMay = glm::translate(modelMatrixRayMay, glm::vec3(0, 1, 0));
		float maxDistanceRay = 10.0;
		glm::vec3 rayDirection = modelMatrixRayMay[2];
		glm::vec3 ori = modelMatrixRayMay[3];
		glm::vec3 rmd = ori + rayDirection * (maxDistanceRay / 2.0f);
		glm::vec3 targetRay = ori + rayDirection * maxDistanceRay;
		modelMatrixRayMay[3] = glm::vec4(rmd, 1.0);
		modelMatrixRayMay = glm::rotate(modelMatrixRayMay, glm::radians(90.0f), 
			glm::vec3(1, 0, 0));
		modelMatrixRayMay = glm::scale(modelMatrixRayMay, 
			glm::vec3(0.05, maxDistanceRay, 0.05));
		rayModel.render(modelMatrixRayMay);

		std::map<std::string, std::tuple<AbstractModel::SBB, glm::mat4, glm::mat4>>::
			iterator itSBB;
		for (itSBB = collidersSBB.begin(); itSBB != collidersSBB.end(); itSBB++) {
			float tRint;
			if (raySphereIntersect(ori, targetRay, rayDirection,
				std::get<0>(itSBB->second), tRint)) {
				std::cout << "Collision del rayo con el modelo " << itSBB->first 
				<< std::endl;
			}
		}
		/*std::map<std::string, std::tuple<AbstractModel::OBB, glm::mat4, glm::mat4>>::
			iterator itOBB;
		for (itOBB = collidersOBB.begin(); itOBB != collidersOBB.end(); itOBB++) {
			if (testRayOBB(ori, targetRay, std::get<0>(itOBB->second))) {
				std::cout << "Collision del rayo con el modelo " << itOBB->first
					<< std::endl;
			}
		}*/

		// Esto es para ilustrar la transformacion inversa de los coliders
		/*glm::vec3 cinv = glm::inverse(mayowCollider.u) * glm::vec4(rockCollider.c, 1.0);
		glm::mat4 invColliderS = glm::mat4(1.0);
		invColliderS = glm::translate(invColliderS, cinv);
		invColliderS =  invColliderS * glm::mat4(mayowCollider.u);
		invColliderS = glm::scale(invColliderS, glm::vec3(rockCollider.ratio * 2.0, rockCollider.ratio * 2.0, rockCollider.ratio * 2.0));
		sphereCollider.setColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
		sphereCollider.enableWireMode();
		sphereCollider.render(invColliderS);
		glm::vec3 cinv2 = glm::inverse(mayowCollider.u) * glm::vec4(mayowCollider.c, 1.0);
		glm::mat4 invColliderB = glm::mat4(1.0);
		invColliderB = glm::translate(invColliderB, cinv2);
		invColliderB = glm::scale(invColliderB, mayowCollider.e * 2.0f);
		boxCollider.setColor(glm::vec4(1.0, 1.0, 0.0, 1.0));
		boxCollider.enableWireMode();
		boxCollider.render(invColliderB);
		// Se regresa el color blanco
		sphereCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));
		boxCollider.setColor(glm::vec4(1.0, 1.0, 1.0, 1.0));*/
		
		/**********Maquinas de estado*************/
		
		

		// Constantes de animaciones
		animationMayowIndex = 1;

		glfwSwapBuffers(window);

		/****************************+
		 * Open AL sound data
		 */
		/*source0Pos[0] = modelMatrixFountain[3].x;
		source0Pos[1] = modelMatrixFountain[3].y;
		source0Pos[2] = modelMatrixFountain[3].z;
		alSourcefv(source[0], AL_POSITION, source0Pos);

		source1Pos[0] = modelMatrixGuardian[3].x;
		source1Pos[1] = modelMatrixGuardian[3].y;
		source1Pos[2] = modelMatrixGuardian[3].z;
		alSourcefv(source[1], AL_POSITION, source1Pos);
		
		source2Pos[0] = modelMatrixDart[3].x;
		source2Pos[1] = modelMatrixDart[3].y;
		source2Pos[2] = modelMatrixDart[3].z;
		alSourcefv(source[2], AL_POSITION, source2Pos);*/

		// Listener for the Thris person camera
		listenerPos[0] = modelMatrixMayow[3].x;
		listenerPos[1] = modelMatrixMayow[3].y;
		listenerPos[2] = modelMatrixMayow[3].z;
		alListenerfv(AL_POSITION, listenerPos);

		glm::vec3 upModel = glm::normalize(modelMatrixMayow[1]);
		glm::vec3 frontModel = glm::normalize(modelMatrixMayow[2]);

		listenerOri[0] = frontModel.x;
		listenerOri[1] = frontModel.y;
		listenerOri[2] = frontModel.z;
		listenerOri[3] = upModel.x;
		listenerOri[4] = upModel.y;
		listenerOri[5] = upModel.z;

		// Listener for the First person camera
		// listenerPos[0] = camera->getPosition().x;
		// listenerPos[1] = camera->getPosition().y;
		// listenerPos[2] = camera->getPosition().z;
		// alListenerfv(AL_POSITION, listenerPos);
		// listenerOri[0] = camera->getFront().x;
		// listenerOri[1] = camera->getFront().y;
		// listenerOri[2] = camera->getFront().z;
		// listenerOri[3] = camera->getUp().x;
		// listenerOri[4] = camera->getUp().y;
		// listenerOri[5] = camera->getUp().z;
		alListenerfv(AL_ORIENTATION, listenerOri);

		for(unsigned int i = 0; i < sourcesPlay.size(); i++){
			if(sourcesPlay[i]){
				sourcesPlay[i] = false;
				alSourcePlay(source[i]);
			}
		}
	}
}

int main(int argc, char **argv) {
	init(800, 700, "Window GLFW", false);
	applicationLoop();
	destroy();
	return 1;
}