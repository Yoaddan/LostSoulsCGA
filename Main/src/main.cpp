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
Box boxCollider;
Sphere sphereCollider(10, 10);
Cylinder rayModel(10, 10, 1.0, 1.0, 1.0);
Box boxIntro;
Box boxViewDepth;

// Modelos externos
Model laberinto; //Laberinto principal.
Model fantasma; //Enemigo que atraviesa paredes.
Model guardia1; //Enemigo con ruta predefinida.
Model guardia2;
Model guardia3;
Model modelAntorcha; //Antorchas.
Model tesoro;
Model fuego; //Fuego animado.
Model hada; //Hada.

// Personaje principal
Model modelMainCharacter;

// Terrain model instance
Terrain terrain(-1, -1, 200, 5, "../Textures/Height Map/heightmap.png");

// Apuntador shadow box
ShadowBox * shadowBox;

GLuint textureTerrainRID, textureTerrainGID, textureTerrainBID, textureTerrainBlendMapID;
GLuint skyboxTextureID;
GLuint textureInit1ID, textureInit2ID, textureActivaID, textureScreenID;
GLuint textureCounter0ID, textureCounter1ID, textureCounter2ID, textureCounter3ID;

int treasuresCollected = 0;
//std::vector<int> treasuresToRemove;
std::vector<std::string> treasuresToRemove;
std::vector<int> indicesToRemove;
std::vector<std::string> collidersToRemove;
std::map<std::string, bool> collisionDetection;


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
glm::mat4 modelMatrixGuardia1 = glm::mat4(1.0f);
glm::mat4 modelMatrixGuardia2 = glm::mat4(1.0f);
glm::mat4 modelMatrixGuardia3 = glm::mat4(1.0f);
glm::mat4 modelMatrixHada = glm::mat4(1.0f);

glm::mat4 modelMatrixMainCharacter = glm::mat4(1.0f);

int animationMainCharacterIndex = 1;

// Posicion antorchas
std::vector<glm::vec3> torchesPosition = {
	glm::vec3(-1.79279, 0, 2.06897),
	glm::vec3(-2.1736, 0, 36.6826),
	glm::vec3(36.1438, 0, -1.00821),
	glm::vec3(9.27195, 0, -21.005),
	glm::vec3(-31.1512, 0, -12.7198),
	glm::vec3(-37.0462, 0, 16.0555)
};
// Orientación antorchas.
std::vector<float> torchesOrientation = {
	0.0, 90.0, 180.0, 270.0, 0.0, 90.0
};

//Posicion tesoros
std::vector<glm::vec3> tesorosPosition = {
	glm::vec3(29.8276, 0, 44.1803), // 1er tesoro
	glm::vec3(36.4495, 0, 47.1476), // 2do tesoro
	glm::vec3(44.9195, 0,42.7527)   // 3er tesoro
};
// Orientación tesoros
std::vector<float> tesorosOrientation = {
	0.0, -90.0, 180.0
};

std::vector<bool> tesorosRecogidos(tesorosPosition.size(), false);


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
ALfloat source0Pos[] = { 0.0, 0.0, 0.0 };
ALfloat source0Vel[] = { 0.0, 0.0, 0.0 };
// Source 1
ALfloat source1Pos[] = { 0.0, 0.0, 0.0 };
ALfloat source1Vel[] = { 0.0, 0.0, 0.0 };
// Source 2
ALfloat source2Pos[] = { 0.0, 0.0, 0.0 };
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
	guardia1.loadModel("../models/Guardia/c1530.obj");
	guardia1.setShader(&shaderMulLighting);
	guardia2.loadModel("../models/Guardia/c1530.obj");
	guardia2.setShader(&shaderMulLighting);
	guardia3.loadModel("../models/Guardia/c1530.obj");
	guardia3.setShader(&shaderMulLighting);

	// Antorcha
	modelAntorcha.loadModel("../models/Antorcha/Antorcha.obj");	
	modelAntorcha.setShader(&shaderMulLighting);

	// Tesoro
	tesoro.loadModel("../models/Chest/Chest.obj");
	tesoro.setShader(&shaderMulLighting);

	// Fuego
	fuego.loadModel("../models/Fuego/Fuego.fbx");
	fuego.setShader(&shaderMulLighting);

	// Hada
	hada.loadModel("../models/Hada/Hada.obj");
	hada.setShader(&shaderMulLighting);
	
	// Personaje principal
	modelMainCharacter.loadModel("../models/PersonajePrincipal/PersonajePrincipal.obj");
	modelMainCharacter.setShader(&shaderMulLighting);

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
	Texture textureScreen("../Textures/Counter0.png");
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

	// Definiendo las texturas de la GUI
	Texture textureCounter0("../Textures/Counter0.png");
	textureCounter0.loadImage(); // Cargar la textura
	glGenTextures(1, &textureCounter0ID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureCounter0ID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureCounter0.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureCounter0.getChannels() == 3 ? GL_RGB : GL_RGBA, textureCounter0.getWidth(), textureIntro2.getHeight(), 0,
		textureCounter0.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureCounter0.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureCounter0.freeImage(); // Liberamos memoria

	Texture textureCounter1("../Textures/Counter1.png");
	textureCounter1.loadImage(); // Cargar la textura
	glGenTextures(1, &textureCounter1ID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureCounter1ID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureCounter1.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureCounter1.getChannels() == 3 ? GL_RGB : GL_RGBA, textureCounter1.getWidth(), textureIntro2.getHeight(), 0,
		textureCounter1.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureCounter1.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureCounter1.freeImage(); // Liberamos memoria

	Texture textureCounter2("../Textures/Counter2.png");
	textureCounter2.loadImage(); // Cargar la textura
	glGenTextures(1, &textureCounter2ID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureCounter2ID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureCounter2.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureCounter2.getChannels() == 3 ? GL_RGB : GL_RGBA, textureCounter2.getWidth(), textureIntro2.getHeight(), 0,
		textureCounter2.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureCounter2.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureCounter2.freeImage(); // Liberamos memoria

	Texture textureCounter3("../Textures/Counter3.png");
	textureCounter3.loadImage(); // Cargar la textura
	glGenTextures(1, &textureCounter3ID); // Creando el id de la textura del landingpad
	glBindTexture(GL_TEXTURE_2D, textureCounter3ID); // Se enlaza la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // Wrapping en el eje u
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // Wrapping en el eje v
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Filtering de minimización
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Filtering de maximimizacion
	if(textureCounter3.getData()){
		// Transferir los datos de la imagen a la tarjeta
		glTexImage2D(GL_TEXTURE_2D, 0, textureCounter3.getChannels() == 3 ? GL_RGB : GL_RGBA, textureCounter3.getWidth(), textureIntro2.getHeight(), 0,
		textureCounter3.getChannels() == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, textureCounter3.getData());
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else 
		std::cout << "Fallo la carga de textura" << std::endl;
	textureCounter3.freeImage(); // Liberamos memoria

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
	buffer[0] = alutCreateBufferFromFile("../sounds/fantasma.wav");
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
	alSourcef(source[0], AL_GAIN, 0.3f);
	alSourcefv(source[0], AL_POSITION, source0Pos);
	alSourcefv(source[0], AL_VELOCITY, source0Vel);
	alSourcei(source[0], AL_BUFFER, buffer[0]);
	alSourcei(source[0], AL_LOOPING, AL_TRUE);
    alSourcef(source[0], AL_MAX_DISTANCE, 0.00000000000000000000000000001f);       // Distancia máxima en la que el sonido se escucha

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
	guardia1.destroy();
	guardia2.destroy();
	guardia3.destroy();
	modelAntorcha.destroy();
	tesoro.destroy();
	fuego.destroy();
	hada.destroy();

	modelMainCharacter.destroy();

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
	glDeleteTextures(1, &textureCounter0ID);
	glDeleteTextures(1, &textureCounter1ID);
	glDeleteTextures(1, &textureCounter2ID);
	glDeleteTextures(1, &textureCounter3ID);

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
	} else {
		switch(treasuresCollected) {
			case 0:
				textureActivaID = textureCounter0ID;
				break;
			case 1:
				textureActivaID = textureCounter1ID;
				break;
			case 2:
				textureActivaID = textureCounter2ID;
				break;
			case 3:
				textureActivaID = textureCounter3ID;
				break;
			default:
				std::cout << "Número de tesoros fuera de rango." << std::endl;
				break;
		}
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
			modelMatrixMainCharacter = glm::translate(modelMatrixMainCharacter, glm::vec3(0, 0, -axes[1] * 0.1));
			animationMainCharacterIndex = 0;
		}if(fabs(axes[0]) > 0.2){
			modelMatrixMainCharacter = glm::rotate(modelMatrixMainCharacter, glm::radians(-axes[0] * 1.5f), glm::vec3(0, 1, 0));
			animationMainCharacterIndex = 0;
		}

		if(fabs(axes[2]) > 0.2){
			camera->mouseMoveCamera(axes[2], 0.0, deltaTime);
		}if(fabs(axes[5]) > 0.2){
			camera->mouseMoveCamera(0.0, axes[5], deltaTime);
		}

		const unsigned char * buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
		std::cout << "Número de botones disponibles :=>" << buttonCount << std::endl;
		if(buttons[0] == GLFW_PRESS)
			std::cout << "Se presiona cuadrado" << std::endl;

	}

	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		camera->mouseMoveCamera(offsetX, 0.0, deltaTime);
	if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		camera->mouseMoveCamera(0.0, offsetY, deltaTime);

	offsetX = 0;
	offsetY = 0;

	// Controles de personaje principal
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
		modelMatrixMainCharacter = glm::rotate(modelMatrixMainCharacter, 0.10f, glm::vec3(0, 1, 0));
		animationMainCharacterIndex = 0;
	} else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
		modelMatrixMainCharacter = glm::rotate(modelMatrixMainCharacter, -0.10f, glm::vec3(0, 1, 0));
		animationMainCharacterIndex = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
		modelMatrixMainCharacter = glm::translate(modelMatrixMainCharacter, glm::vec3(0.0, 0.0, 0.5));
		animationMainCharacterIndex = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
		modelMatrixMainCharacter = glm::translate(modelMatrixMainCharacter, glm::vec3(0.0, 0.0,-0.5));
		animationMainCharacterIndex = 0;
	}

	glfwPollEvents();
	return continueApplication;
}

void prepareScene(){

	terrain.setShader(&shaderTerrain);

	// Fuego
	fuego.setShader(&shaderMulLighting);

	// Hada
	hada.setShader(&shaderMulLighting);

	// Antorcha
	modelAntorcha.setShader(&shaderMulLighting);

	// Tesoro
	tesoro.setShader(&shaderMulLighting);

	//Mayow
	modelMainCharacter.setShader(&shaderMulLighting);

	// Laberinto
	laberinto.setShader(&shaderMulLighting);

	// Fantasma
	fantasma.setShader(&shaderMulLighting);

	// Guardia
	guardia1.setShader(&shaderMulLighting);
	guardia2.setShader(&shaderMulLighting);
	guardia3.setShader(&shaderMulLighting);

}

void prepareDepthScene(){

	terrain.setShader(&shaderDepth);

	// Fuego
	fuego.setShader(&shaderDepth);

	// Hada
	hada.setShader(&shaderDepth);

	// Antorcha
	modelAntorcha.setShader(&shaderDepth);

	// Tesoro
	tesoro.setShader(&shaderDepth);

	//Mayow
	modelMainCharacter.setShader(&shaderDepth);

	// Laberinto
	laberinto.setShader(&shaderDepth);

	// Fantasma
	fantasma.setShader(&shaderDepth);

	// Guardia
	guardia1.setShader(&shaderDepth);
	guardia2.setShader(&shaderDepth);
	guardia3.setShader(&shaderDepth);

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

	// Render antorchas con fuego
	for(int i = 0; i < torchesPosition.size(); i++){
		torchesPosition[i].y = terrain.getHeightTerrain(torchesPosition[i].x, torchesPosition[i].z);
		modelAntorcha.setPosition(torchesPosition[i]);
		modelAntorcha.setScale(glm::vec3(1.0));
		modelAntorcha.setOrientation(glm::vec3(0, torchesOrientation[i], 0));
		modelAntorcha.render();
		glm::vec3 fuegoPosition = torchesPosition[i];
		fuegoPosition.y = torchesPosition[i].y + 1.05;
		fuego.setPosition(fuegoPosition);
		fuego.setScale(glm::vec3(0.003,0.003,0.003));
		fuego.render();
	}
	
	// Renderizar solo los tesoros que no han sido recogidos
	for (int i = 0; i < tesorosPosition.size(); ++i) {
		if (!tesorosRecogidos[i]) {
			tesorosPosition[i].y = terrain.getHeightTerrain(tesorosPosition[i].x, tesorosPosition[i].z);
			tesoro.setPosition(tesorosPosition[i]);
			tesoro.setScale(glm::vec3(1.0));
			tesoro.setOrientation(glm::vec3(0, tesorosOrientation[i], 0));
			tesoro.render();
		}
	}





	/*****************************************
	 * Objetos animados por huesos
	 * **************************************/
	glm::vec3 ejey = glm::normalize(terrain.getNormalTerrain(modelMatrixMainCharacter[3][0], modelMatrixMainCharacter[3][2]));
	glm::vec3 ejex = glm::vec3(modelMatrixMainCharacter[0]);
	glm::vec3 ejez = glm::normalize(glm::cross(ejex, ejey));
	ejex = glm::normalize(glm::cross(ejey, ejez));
	modelMatrixMainCharacter[0] = glm::vec4(ejex, 0.0);
	modelMatrixMainCharacter[1] = glm::vec4(ejey, 0.0);
	modelMatrixMainCharacter[2] = glm::vec4(ejez, 0.0);
	modelMatrixMainCharacter[3][1] = terrain.getHeightTerrain(modelMatrixMainCharacter[3][0], modelMatrixMainCharacter[3][2]);
	glm::mat4 modelMatrixMainCharacterBody = glm::mat4(modelMatrixMainCharacter);
	//modelMatrixMainCharacterBody = glm::scale(modelMatrixMainCharacterBody, glm::vec3(0.021f)); //Se debe comentar despues
	modelMainCharacter.setAnimationIndex(animationMainCharacterIndex);
	modelMainCharacter.render(modelMatrixMainCharacterBody);

	modelMatrixHada = modelMatrixMainCharacter;
	modelMatrixHada = glm::translate(modelMatrixHada, glm::vec3(0.174625f,1.552f, -0.5f));
	hada.render(modelMatrixHada);

	modelMatrixGuardia1[3][1] = terrain.getHeightTerrain(modelMatrixGuardia1[3][0], modelMatrixGuardia1[3][2]);
	glm::mat4 modelMatrixGuardia1Body = glm::mat4(modelMatrixGuardia1);
	guardia1.render(modelMatrixGuardia1Body);

	modelMatrixGuardia2[3][1] = terrain.getHeightTerrain(modelMatrixGuardia2[3][0], modelMatrixGuardia2[3][2]);
	glm::mat4 modelMatrixGuardia2Body = glm::mat4(modelMatrixGuardia2);
	guardia2.render(modelMatrixGuardia2Body);

	modelMatrixGuardia3[3][1] = terrain.getHeightTerrain(modelMatrixGuardia3[3][0], modelMatrixGuardia3[3][2]);
	glm::mat4 modelMatrixGuardia3Body = glm::mat4(modelMatrixGuardia3);
	guardia3.render(modelMatrixGuardia3Body);

	// Interpolación lineal para mover el fantasma hacia la posición de Mayow
	float interpolationFactor = 0.001f; // Ajusta este valor para cambiar la velocidad de seguimiento
	glm::vec3 newPositionFantasma = glm::mix(glm::vec3(modelMatrixFantasma[3]), glm::vec3(modelMatrixMainCharacter[3]), interpolationFactor);

	// Calcular la dirección hacia Mayow y ajustar la orientación del fantasma
	glm::vec3 directionFantasma = glm::normalize(glm::vec3(modelMatrixMainCharacter[3]) - newPositionFantasma);
	glm::vec3 upFantasma = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 rightFantasma = glm::normalize(glm::cross(upFantasma, directionFantasma));
	upFantasma = glm::cross(directionFantasma, rightFantasma);

	glm::mat4 rotationMatrixFantasma = glm::mat4(1.0f);
	rotationMatrixFantasma[0] = glm::vec4(rightFantasma, 0.0f);
	rotationMatrixFantasma[1] = glm::vec4(upFantasma, 0.0f);
	rotationMatrixFantasma[2] = glm::vec4(directionFantasma, 0.0f);
/*
	// Actualizar la posición y orientación del fantasma
	modelMatrixFantasma[3] = glm::vec4(newPositionFantasma, 1.0f);
	modelMatrixFantasma = glm::translate(glm::mat4(1.0f), newPositionFantasma) * rotationMatrixFantasma;

*/
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

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if(render) {
        shaderTexture.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
        shaderTexture.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureScreenID);
        shaderTexture.setInt("outTexture", 0);

        boxIntro.render();
        modelText->render("Texto en OpenGL", -1, 0);
    }

    glDisable(GL_BLEND);


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

	int stateG1 = 0;
	float avanceG1 = 0.05;
	float maxAvanceG1 = 0.0;
	float contAvanceG1 =0.0;
	float giroG1 = 1.0;
	int posG1 = 0;

	int stateG2 = 0;
	float avanceG2 = 0.05;
	float maxAvanceG2 = 0.0;
	float contAvanceG2 =0.0;
	float giroG2 = 1.0;
	int posG2 = 0;

	int stateG3 = 0;
	float avanceG3 = 0.05;
	float maxAvanceG3 = 0.0;
	float contAvanceG3 =0.0;
	float giroG3 = 1.0;
	int posG3 = 0;

	// Posiciones y estados iniciales.
	modelMatrixMainCharacter = glm::translate(modelMatrixMainCharacter, glm::vec3(34.677f, 0.05f, 37.0987f));
	//modelMatrixMainCharacter = glm::translate(modelMatrixMainCharacter, glm::vec3(25.0f, 0.05f, 0.7f));
	modelMatrixMainCharacter = glm::rotate(modelMatrixMainCharacter, glm::radians(-90.0f), glm::vec3(0, 1, 0));

	//Guardias
	modelMatrixGuardia1 = glm::translate(modelMatrixGuardia1, glm::vec3(29.0f, 0.05f, -5.0f));
	modelMatrixGuardia1 = glm::rotate(modelMatrixGuardia1, glm::radians(180.0f), glm::vec3(0, 1, 0));
	modelMatrixGuardia1 = glm::scale(modelMatrixGuardia1, glm::vec3(1.45));

	modelMatrixGuardia2 = glm::translate(modelMatrixGuardia2, glm::vec3(-28.0f, 0.05f, 35.0f));
	modelMatrixGuardia2 = glm::rotate(modelMatrixGuardia2, glm::radians(-90.0f), glm::vec3(0, 1, 0));
	modelMatrixGuardia2 = glm::scale(modelMatrixGuardia2, glm::vec3(1.45));

	modelMatrixGuardia3 = glm::translate(modelMatrixGuardia3, glm::vec3(-18.0f, 0.05f, 0.7f));
	modelMatrixGuardia3 = glm::rotate(modelMatrixGuardia3, glm::radians(90.0f), glm::vec3(0, 1, 0));
	modelMatrixGuardia3 = glm::scale(modelMatrixGuardia3, glm::vec3(1.45));

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

		
		axis = glm::axis(glm::quat_cast(modelMatrixMainCharacter));
		angleTarget = glm::angle(glm::quat_cast(modelMatrixMainCharacter));
		target = modelMatrixMainCharacter[3];
		
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
		shaderMulLighting.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderMulLighting.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
		shaderMulLighting.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.7071, -0.7071, -0.7071)));

		shaderTerrain.setVectorFloat3("viewPos", glm::value_ptr(camera->getPosition()));
		shaderTerrain.setVectorFloat3("directionalLight.light.ambient", glm::value_ptr(glm::vec3(0.5, 0.5, 0.5)));
		shaderTerrain.setVectorFloat3("directionalLight.light.diffuse", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderTerrain.setVectorFloat3("directionalLight.light.specular", glm::value_ptr(glm::vec3(0.0, 0.0, 0.0)));
		shaderTerrain.setVectorFloat3("directionalLight.direction", glm::value_ptr(glm::vec3(-0.7071, -0.7071, -0.7071)));
		
		/*******************************************
		 * Propiedades PointLights
		 *******************************************/
		shaderMulLighting.setInt("pointLightCount", torchesPosition.size()+1);
		shaderTerrain.setInt("pointLightCount", torchesPosition.size()+1);
		for(int i = 0; i < torchesPosition.size(); i++){
			glm::mat4 matrixAdjustTorch = glm::mat4(1.0);
			matrixAdjustTorch = glm::translate(matrixAdjustTorch, torchesPosition[i]);
			matrixAdjustTorch = glm::rotate(matrixAdjustTorch, glm::radians(torchesOrientation[i]), glm::vec3(0, 1, 0));
			matrixAdjustTorch = glm::scale(matrixAdjustTorch, glm::vec3(1.0));
			matrixAdjustTorch = glm::translate(matrixAdjustTorch, glm::vec3(0.0, 0.929738, 0));
			glm::vec3 torchPosition = glm::vec3(matrixAdjustTorch[3]);
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.2, 0.16, 0.01)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.4, 0.32, 0.02)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(i) + "].position", glm::value_ptr(torchPosition));
			shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0);
			shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09);
			shaderMulLighting.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.02);
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.ambient", glm::value_ptr(glm::vec3(0.2, 0.16, 0.01)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.diffuse", glm::value_ptr(glm::vec3(0.4, 0.32, 0.02)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].light.specular", glm::value_ptr(glm::vec3(0.6, 0.58, 0.03)));
			shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(i) + "].position", glm::value_ptr(torchPosition));
			shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0);
			shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.09);
			shaderTerrain.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.02);
		}

		// Pointlight del hada
		glm::mat4 matrixAdjustFairy = glm::mat4(1.0);
		matrixAdjustFairy = glm::translate(matrixAdjustFairy, glm::vec3(modelMatrixHada[3]));
		glm::vec3 FairyPosition = glm::vec3(matrixAdjustFairy[3]);
		shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].light.ambient", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].light.diffuse", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].light.specular", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderMulLighting.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].position", glm::value_ptr(FairyPosition));
		shaderMulLighting.setFloat("pointLights[" + std::to_string(torchesPosition.size()) + "].constant", 1.0);
		shaderMulLighting.setFloat("pointLights[" + std::to_string(torchesPosition.size()) + "].linear", 0.09);
		shaderMulLighting.setFloat("pointLights[" + std::to_string(torchesPosition.size()) + "].quadratic", 0.02);
		shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].light.ambient", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].light.diffuse", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].light.specular", glm::value_ptr(glm::vec3(0.1, 0.1, 0.1)));
		shaderTerrain.setVectorFloat3("pointLights[" + std::to_string(torchesPosition.size()) + "].position", glm::value_ptr(FairyPosition));
		shaderTerrain.setFloat("pointLights[" + std::to_string(torchesPosition.size()) + "].constant", 1.0);
		shaderTerrain.setFloat("pointLights[" + std::to_string(torchesPosition.size()) + "].linear", 0.09);
		shaderTerrain.setFloat("pointLights[" + std::to_string(torchesPosition.size()) + "].quadratic", 0.02);

		// Para sombras
		shaderTerrain.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));
		shaderMulLighting.setMatrix4("lightSpaceMatrix", 1, false, glm::value_ptr(lightSpaceMatrix));

		// Render del buffer de profundidad desde el punto de vista de la luz
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Configuración del viewport para el mapa de sombras
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Preparar la escena para el renderizado del mapa de profundidad
		prepareDepthScene();
		renderScene();

		// Desvincular el framebuffer después de renderizar
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Render normal de los objetos
		glViewport(0, 0, screenWidth, screenHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		prepareScene();
		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		shaderMulLighting.setInt("shadowMap", 10);
		shaderTerrain.setInt("shadowMap", 10);
		renderSolidScene();

		// Habilitar blending para transparencia
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Deshabilitar depth test para el GUI
		glDisable(GL_DEPTH_TEST);

		/************ Render de imagen de frente **************/    
		shaderTexture.setMatrix4("projection", 1, false, glm::value_ptr(glm::mat4(1.0)));
		shaderTexture.setMatrix4("view", 1, false, glm::value_ptr(glm::mat4(1.0)));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureActivaID);
		shaderTexture.setInt("outTexture", 0);
		boxIntro.render();
		//glfwSwapBuffers(window);

		// Restaurar el estado después de renderizar el GUI
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);

		
		/*
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
		*/


		/*******************************************
		 * Creacion de colliders
		 * IMPORTANT do this before interpolations
		 *******************************************/

		// Antorchas colliders
		for (int i = 0; i < torchesPosition.size(); i++){
			AbstractModel::OBB lampCollider;
			glm::mat4 modelMatrixColliderLamp = glm::mat4(1.0);
			modelMatrixColliderLamp = glm::translate(modelMatrixColliderLamp, torchesPosition[i]);
			modelMatrixColliderLamp = glm::rotate(modelMatrixColliderLamp, glm::radians(torchesOrientation[i]),
					glm::vec3(0, 1, 0));
			addOrUpdateColliders(collidersOBB, "Antorcha-" + std::to_string(i), lampCollider, modelMatrixColliderLamp);
			// Set the orientation of collider before doing the scale
			lampCollider.u = glm::quat_cast(modelMatrixColliderLamp);
			modelMatrixColliderLamp = glm::scale(modelMatrixColliderLamp, glm::vec3(1.0, 1.0, 1.0));
			modelMatrixColliderLamp = glm::translate(modelMatrixColliderLamp, modelAntorcha.getObb().c);
			lampCollider.c = glm::vec3(modelMatrixColliderLamp[3]);
			lampCollider.e = modelAntorcha.getObb().e * glm::vec3(1.0, 1.0, 1.0);
			std::get<0>(collidersOBB.find("Antorcha-" + std::to_string(i))->second) = lampCollider;
		}


		// Configurar los colliders de los tesoros
		for (int i = 0; i < tesorosPosition.size(); i++) {
			// Configurar el collider del tesoro
			AbstractModel::OBB tesoroCollider;
			glm::mat4 modelMatrixColliderTesoro = glm::mat4(1.0);
			modelMatrixColliderTesoro = glm::translate(modelMatrixColliderTesoro, tesorosPosition[i]);
			modelMatrixColliderTesoro = glm::rotate(modelMatrixColliderTesoro, glm::radians(tesorosOrientation[i]), glm::vec3(0, 1, 0));
			addOrUpdateColliders(collidersOBB, "Tesoro-" + std::to_string(i), tesoroCollider, modelMatrixColliderTesoro);
			tesoroCollider.u = glm::quat_cast(modelMatrixColliderTesoro);
			modelMatrixColliderTesoro = glm::scale(modelMatrixColliderTesoro, glm::vec3(1.0, 1.0, 1.0));
			modelMatrixColliderTesoro = glm::translate(modelMatrixColliderTesoro, tesoro.getObb().c);
			tesoroCollider.c = glm::vec3(modelMatrixColliderTesoro[3]);
			tesoroCollider.e = tesoro.getObb().e * glm::vec3(1.0, 1.0, 1.0);
			std::get<0>(collidersOBB.find("Tesoro-" + std::to_string(i))->second) = tesoroCollider;
		}


		// Define una estructura para mantener la información de cada collider
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


		// Collider de personaje principal
		AbstractModel::OBB mainCharacterCollider;
		glm::mat4 modelmatrixColliderMainCharacter = glm::mat4(modelMatrixMainCharacter);
		// Set the orientation of collider before doing the scale
		mainCharacterCollider.u = glm::quat_cast(modelmatrixColliderMainCharacter);
		modelmatrixColliderMainCharacter = glm::scale(modelmatrixColliderMainCharacter, glm::vec3(1.0, 1.0, 1.0));
		modelmatrixColliderMainCharacter = glm::translate(modelmatrixColliderMainCharacter,
				glm::vec3(modelMainCharacter.getObb().c.x,
						modelMainCharacter.getObb().c.y,
						modelMainCharacter.getObb().c.z));
		mainCharacterCollider.e = modelMainCharacter.getObb().e * glm::vec3(1.0, 1.0, 1.01) * glm::vec3(0.787401574, 0.787401574, 0.787401574);
		mainCharacterCollider.c = glm::vec3(modelmatrixColliderMainCharacter[3]);
		addOrUpdateColliders(collidersOBB, "main", mainCharacterCollider, modelMatrixMainCharacter);

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
//abcdef
		//Collider Guardia 1
		AbstractModel::OBB coliderGuardia1;
		glm::mat4 modelMatrixColliderGuardia1 = glm::mat4(modelMatrixGuardia1);
		// Set the orientation of collider before doing the scale
		coliderGuardia1.u = glm::quat_cast(modelMatrixColliderGuardia1);
		modelMatrixColliderGuardia1 = glm::scale(modelMatrixColliderGuardia1, glm::vec3(1.0, 1.0, 1.0));
		modelMatrixColliderGuardia1 = glm::translate(modelMatrixColliderGuardia1,
				glm::vec3(guardia1.getObb().c.x,
						guardia1.getObb().c.y,
						guardia1.getObb().c.z));
		coliderGuardia1.e = guardia1.getObb().e * glm::vec3(1.0, 1.0, 1.0);
		coliderGuardia1.c = glm::vec3(modelMatrixColliderGuardia1[3]);
		addOrUpdateColliders(collidersOBB, "Guardia1", coliderGuardia1, modelMatrixGuardia1);

				//Collider Guardia 2
		AbstractModel::OBB coliderGuardia2;
		glm::mat4 modelMatrixColliderGuardia2 = glm::mat4(modelMatrixGuardia2);
		// Set the orientation of collider before doing the scale
		coliderGuardia2.u = glm::quat_cast(modelMatrixColliderGuardia2);
		modelMatrixColliderGuardia2 = glm::scale(modelMatrixColliderGuardia2, glm::vec3(1.0, 1.0, 1.0));
		modelMatrixColliderGuardia2 = glm::translate(modelMatrixColliderGuardia2,
				glm::vec3(guardia2.getObb().c.x,
						guardia2.getObb().c.y,
						guardia2.getObb().c.z));
		coliderGuardia2.e = guardia2.getObb().e * glm::vec3(1.0, 1.0, 1.0);
		coliderGuardia2.c = glm::vec3(modelMatrixColliderGuardia2[3]);
		addOrUpdateColliders(collidersOBB, "Guardia2", coliderGuardia2, modelMatrixGuardia2);

				//Collider Guardia 3
		AbstractModel::OBB coliderGuardia3;
		glm::mat4 modelMatrixColliderGuardia3 = glm::mat4(modelMatrixGuardia1);
		// Set the orientation of collider before doing the scale
		coliderGuardia3.u = glm::quat_cast(modelMatrixColliderGuardia3);
		modelMatrixColliderGuardia3 = glm::scale(modelMatrixColliderGuardia3, glm::vec3(1.0, 1.0, 1.0));
		modelMatrixColliderGuardia3 = glm::translate(modelMatrixColliderGuardia3,
				glm::vec3(guardia3.getObb().c.x,
						guardia3.getObb().c.y,
						guardia3.getObb().c.z));
		coliderGuardia3.e = guardia3.getObb().e * glm::vec3(1.0, 1.0, 1.0);
		coliderGuardia3.c = glm::vec3(modelMatrixColliderGuardia3[3]);
		addOrUpdateColliders(collidersOBB, "Guardia3", coliderGuardia3, modelMatrixGuardia3);

		/*******************************************
		 * Render de colliders
		 *******************************************/
		/*
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
		}*/

		
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



		for (auto it = collidersOBB.begin(); it != collidersOBB.end(); ++it) {
			if (it->first.find("Tesoro-") != std::string::npos) {
				// Verificar si el tesoro ya ha sido recogido
				auto index = std::stoi(it->first.substr(it->first.find('-') + 1));
				if (tesorosRecogidos[index]) {
					continue; // Saltar este tesoro si ya ha sido recogido
				}

				for (auto jt = collidersOBB.begin(); jt != collidersOBB.end(); ++jt) {
					if (jt->first == "main" && testOBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
						std::cout << "El jugador ha recogido un tesoro." << std::endl;
						treasuresCollected++;
						// Marcar el tesoro como recogido
						tesorosRecogidos[index] = true;
						collidersToRemove.push_back(it->first);
						break; // Evitar múltiples detecciones para el mismo tesoro
					}
				}
			} else if (it->first == "fantasma") {
				for (auto jt = collidersOBB.begin(); jt != collidersOBB.end(); ++jt) {
					if (jt->first == "main" && testOBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
						std::cout << "El fantasma ha colisionado con el jugador. Cerrando el juego..." << std::endl;
						exit(1); // Terminar el juego al detectar colisión con el fantasma
					}
				}
			}else if (it->first == "Guardia1" || it->first == "Guardia2" || it->first == "Guardia3") {
				for (auto jt = collidersOBB.begin(); jt != collidersOBB.end(); ++jt) {
					if (jt->first == "main" && testOBBOBB(std::get<0>(it->second), std::get<0>(jt->second))) {
						std::cout << "El Guardia ha colisionado con el jugador. Cerrando el juego..." << std::endl;
						exit(1); // Terminar el juego al detectar colisión con el fantasma
					}
				}
			}
		}		

		// Eliminar los colisionadores de los tesoros recogidos
		for (const auto& collider : collidersToRemove) {
			collidersOBB.erase(collider);
		}

		// Eliminar los tesoros recogidos de los vectores de posición y orientación
		std::sort(indicesToRemove.rbegin(), indicesToRemove.rend());
		for (const auto& index : indicesToRemove) {
			if (index >= 0 && index < tesorosPosition.size()) {
				tesorosPosition.erase(tesorosPosition.begin() + index);
				tesorosOrientation.erase(tesorosOrientation.begin() + index);
			}
		}

		// Actualización de la detección de colisiones
		for (auto& collider : collidersOBB) {
			bool isCollision = false;
			for (auto& otherCollider : collidersOBB) {
				if (collider.first != otherCollider.first && testOBBOBB(std::get<0>(collider.second), std::get<0>(otherCollider.second))) {
					isCollision = true;
					break;
				}
			}
			addOrUpdateCollisionDetection(collisionDetection, collider.first, isCollision);
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
					if (itCollision->first.compare("main") == 0)
						modelMatrixMainCharacter = std::get<1>(obbBuscado->second);
				}
			}
		}

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
		//Guardia 1
		switch (stateG1)
		{
		case 0:
			if(posG1 == 0 || posG1 == 2 || posG1 == 29 || posG1 == 31){
				//printf("%i",posG1);
				maxAvanceG1 = 8.0;
				stateG1 = 1;
			}
			//izquierda
			if(posG1 == 1 || posG1==5 || posG1==9 || posG1 == 15 || posG1 == 16 || posG1 == 18 || posG1 == 20
			|| posG1 == 24 || posG1 == 28){
				stateG1 = 2;
			}
			//Derecha
			if(posG1 == 3 || posG1==7 || posG1==11 || posG1==13 || posG1 == 22 || posG1 == 26
			|| posG1 == 30 || posG1 == 32 || posG1 == 33){
				stateG1 = 3;
			}
			//av
			if (posG1==4 || posG1==6 || posG1==8 || posG1==10 || posG1 == 21 || posG1 == 23 || posG1 == 25
			|| posG1 == 27)
			{
				maxAvanceG1 = 4.0;
				stateG1 = 1;
			}
			if (posG1==12 || posG1 == 19)
			{
				maxAvanceG1 = 4.5;
				stateG1 = 1;
			}
			if (posG1 == 14 || posG1 == 17){
				maxAvanceG1 = 10.0;
				stateG1 = 1;
			}
			
			if(posG1 == 34){
				posG1 = 0;
			}
			break;
		//Avanzar
		case 1:
		modelMatrixGuardia1 = glm::translate(modelMatrixGuardia1, glm::vec3(0.0f, 0.0f, avanceG1));
		contAvanceG1 += avanceG1;
		//printf("%f",contAvanceG1);
		if (contAvanceG1 > maxAvanceG1){
			contAvanceG1 = 0.0;
			stateG1 = 0;
			posG1 ++;
		}
		break;
		//Girlo izquierda
		case 2:
		modelMatrixGuardia1 = glm::rotate(modelMatrixGuardia1, glm::radians(giroG1), glm::vec3(0, 1, 0));
		contAvanceG1 += giroG1;
		if(contAvanceG1 >= 90){
			contAvanceG1 = 0.0;
			stateG1 = 0;
			posG1 ++;
		}
		break;
		//Girlo derecha
		case 3:
		modelMatrixGuardia1 = glm::rotate(modelMatrixGuardia1, glm::radians(-giroG1), glm::vec3(0, 1, 0));
		contAvanceG1 += giroG1;
		if(contAvanceG1 >= 90){
			contAvanceG1 = 0.0;
			stateG1 = 0;
			posG1 ++;
		}
		break;
		default:
			break;
		}

		//Guardia 2
		switch (stateG2)
		{
		case 0:
			if(posG2 == 0 || posG2 == 4 || posG2 == 7 || posG2 == 11){
				maxAvanceG2 = 5.0;
				stateG2 = 1;
			}
			//Derecha
			if(posG2 == 1 || posG2 == 3 || posG2 == 12 || posG2 == 13){
				stateG2 = 3;
			}
			if(posG2 == 2  || posG2 == 9){
				maxAvanceG2 = 12.0;
				stateG2 = 1;
			}
			//Izquierda
			if(posG2 == 5 || posG2 == 6 || posG2 == 8 || posG2 == 10){
				stateG2 = 2;
			}
			if(posG2 == 14){
				posG2 = 0;
			}
			break;
		//Avanzar
		case 1:
		modelMatrixGuardia2 = glm::translate(modelMatrixGuardia2, glm::vec3(0.0f, 0.0f, avanceG2));
		contAvanceG2 += avanceG2;
		if (contAvanceG2 > maxAvanceG2){
			contAvanceG2 = 0.0;
			stateG2 = 0;
			posG2 ++;
		}
		break;
		//Girlo izquierda
		case 2:
		modelMatrixGuardia2 = glm::rotate(modelMatrixGuardia2, glm::radians(giroG2), glm::vec3(0, 1, 0));
		contAvanceG2 += giroG2;
		if(contAvanceG2 >= 90){
			contAvanceG2 = 0.0;
			stateG2 = 0;
			posG2 ++;
		}
		break;
		//Girlo derecha
		case 3:
		modelMatrixGuardia2 = glm::rotate(modelMatrixGuardia2, glm::radians(-giroG2), glm::vec3(0, 1, 0));
		contAvanceG2 += giroG2;
		if(contAvanceG2 >= 90){
			contAvanceG2 = 0.0;
			stateG2 = 0;
			posG2 ++;
		}
		break;
		default:
			break;
		}
		//Guardia 3
		switch (stateG3)
		{
		case 0:
			if(posG3 == 0 || posG3 == 7 ){
				maxAvanceG3 = 8.0;
				stateG3 = 1;
			}
			//Izquierda
			if(posG3 == 1 || posG3 == 8 || posG3 == 9){
				stateG3 = 2;
			}
			if(posG3 == 2 || posG3 == 5){
				maxAvanceG3 = 15.0;
				stateG3 = 1;
			}
			//Derecha
			if(posG3 == 3 || posG3 == 4 || posG3 == 6){
				stateG3 = 3;
			}
			
			if(posG3 == 10){
				posG3 = 0;
			}
			break;
		//Avanzar
		case 1:
		modelMatrixGuardia3 = glm::translate(modelMatrixGuardia3, glm::vec3(0.0f, 0.0f, avanceG3));
		contAvanceG3 += avanceG3;
		if (contAvanceG3 > maxAvanceG3){
			contAvanceG3 = 0.0;
			stateG3 = 0;
			posG3 ++;
		}
		break;
		//Girlo izquierda
		case 2:
		modelMatrixGuardia3 = glm::rotate(modelMatrixGuardia3, glm::radians(giroG3), glm::vec3(0, 1, 0));
		contAvanceG3 += giroG3;
		if(contAvanceG3 >= 90){
			contAvanceG3 = 0.0;
			stateG3 = 0;
			posG3 ++;
		}
		break;
		//Girlo derecha
		case 3:
		modelMatrixGuardia3 = glm::rotate(modelMatrixGuardia3, glm::radians(-giroG3), glm::vec3(0, 1, 0));
		contAvanceG3 += giroG3;
		if(contAvanceG3 >= 90){
			contAvanceG3 = 0.0;
			stateG3 = 0;
			posG3 ++;
		}
		break;
		default:
			break;
		}
		//Para saber la posicion del personaje
		//std::cout << " x " << modelMatrixMainCharacter[3].x << " y " << modelMatrixMainCharacter[3].y << " z " << modelMatrixMainCharacter[3].z << std::endl;

		//Contador de tesoros
		//std::cout << " Tesoros Counter: " << treasuresCollected << std::endl;

		// Constantes de animaciones
		animationMainCharacterIndex = 1;

		glfwSwapBuffers(window);

		
		/****************************+
		 * Open AL sound data
		 */
		source0Pos[0] = modelMatrixFantasma[3].x;
		source0Pos[1] = modelMatrixFantasma[3].y;
		source0Pos[2] = modelMatrixFantasma[3].z;
		alSourcefv(source[0], AL_POSITION, source0Pos);

		// Listener for the Thris person camera
		listenerPos[0] = modelMatrixMainCharacter[3].x;
		listenerPos[1] = modelMatrixMainCharacter[3].y;
		listenerPos[2] = modelMatrixMainCharacter[3].z;
		alListenerfv(AL_POSITION, listenerPos);

		glm::vec3 upModel = glm::normalize(modelMatrixMainCharacter[1]);
		glm::vec3 frontModel = glm::normalize(modelMatrixMainCharacter[2]);

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