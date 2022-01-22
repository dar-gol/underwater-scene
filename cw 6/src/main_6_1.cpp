#define STB_IMAGE_IMPLEMENTATION

#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"
#include "Camera.h"
#include "Particles.h"
#include "SOIL/stb_image_aug.h"

GLuint programColor, programTexture, programSkybox, programParticles;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext, terrainContext, sphereContext;

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
glm::vec3 cameraVertical;
float cameraAngle = 0;

float differenceX = 0.0f;
float differenceY = 0.0f;
float prevPosX = -1.0f;
float prevPosY = -1.0f;
glm::quat rotation = glm::quat(1, 0, 0, 0);
glm::quat quatZ;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::vec3(1.0f, -0.9f, -1.0f);

GLuint textureAsteroid, textureTerrain;

unsigned int cubemapTexture; //skybox
unsigned int skyboxVAO, skyboxVBO; //skybox

std::vector<std::string> faces = {
	"textures/skybox/right.png",
	"textures/skybox/left.png",
	"textures/skybox/top.png",
	"textures/skybox/bottom.png",
	"textures/skybox/front.png",
	"textures/skybox/back.png",
};

float skyboxSize = 10.0f;
float distanceToSkybox = 0.5f;

float skyboxVertices[] = {
	-skyboxSize,  skyboxSize, -skyboxSize,
	-skyboxSize, -skyboxSize, -skyboxSize,
	 skyboxSize, -skyboxSize, -skyboxSize,
	 skyboxSize, -skyboxSize, -skyboxSize,
	 skyboxSize,  skyboxSize, -skyboxSize,
	-skyboxSize,  skyboxSize, -skyboxSize,

	-skyboxSize, -skyboxSize,  skyboxSize,
	-skyboxSize, -skyboxSize, -skyboxSize,
	-skyboxSize,  skyboxSize, -skyboxSize,
	-skyboxSize,  skyboxSize, -skyboxSize,
	-skyboxSize,  skyboxSize,  skyboxSize,
	-skyboxSize, -skyboxSize,  skyboxSize,

	 skyboxSize, -skyboxSize, -skyboxSize,
	 skyboxSize, -skyboxSize,  skyboxSize,
	 skyboxSize,  skyboxSize,  skyboxSize,
	 skyboxSize,  skyboxSize,  skyboxSize,
	 skyboxSize,  skyboxSize, -skyboxSize,
	 skyboxSize, -skyboxSize, -skyboxSize,

	-skyboxSize, -skyboxSize,  skyboxSize,
	-skyboxSize,  skyboxSize,  skyboxSize,
	 skyboxSize,  skyboxSize,  skyboxSize,
	 skyboxSize,  skyboxSize,  skyboxSize,
	 skyboxSize, -skyboxSize,  skyboxSize,
	-skyboxSize, -skyboxSize,  skyboxSize,

	-skyboxSize,  skyboxSize, -skyboxSize,
	 skyboxSize,  skyboxSize, -skyboxSize,
	 skyboxSize,  skyboxSize,  skyboxSize,
	 skyboxSize,  skyboxSize,  skyboxSize,
	-skyboxSize,  skyboxSize,  skyboxSize,
	-skyboxSize,  skyboxSize, -skyboxSize,

	-skyboxSize, -skyboxSize, -skyboxSize,
	-skyboxSize, -skyboxSize,  skyboxSize,
	 skyboxSize, -skyboxSize, -skyboxSize,
	 skyboxSize, -skyboxSize, -skyboxSize,
	-skyboxSize, -skyboxSize,  skyboxSize,
	 skyboxSize, -skyboxSize,  skyboxSize
};

void keyboard(unsigned char key, int x, int y)
{

	float angleSpeed = 0.01f;
	float moveSpeed = 0.1f;
	switch (key)
	{
		case 'z': quatZ = glm::angleAxis(angleSpeed, glm::vec3(0.0, 0.0, 1.0)); break;
		case 'x': quatZ = glm::angleAxis(-angleSpeed, glm::vec3(0.0, 0.0, 1.0)); break;
		case 'w': cameraPos += cameraDir * moveSpeed; break;
		case 's': cameraPos -= cameraDir * moveSpeed; break;
		case 'd': cameraPos += cameraSide * moveSpeed; break;
		case 'a': cameraPos -= cameraSide * moveSpeed; break;
		case 'q': cameraPos += cameraVertical * moveSpeed; break;
		case 'e': cameraPos -= cameraVertical * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	if (prevPosX == -1.0f && prevPosY == -1.0f) {
		prevPosX = x;
		prevPosY = y;
	}
	differenceX = x - prevPosX;
	differenceY = y - prevPosY;
	prevPosX = x;
	prevPosY = y;
}

glm::mat4 createCameraMatrix()
{
	glm::quat quatX = glm::angleAxis(differenceY * 0.01f, glm::vec3(1.0, 0.0, 0.0));
	glm::quat quatY = glm::angleAxis(differenceX * 0.01f, glm::vec3(0.0, 1.0, 0.0));

	rotation = glm::normalize(quatX * quatY * quatZ * rotation);

	cameraDir = inverse(rotation) * glm::vec3(0, 0, -1);
	cameraSide = inverse(rotation) * glm::vec3(1, 0, 0);
	cameraVertical = inverse(rotation) * glm::vec3(0, 1, 0);

	differenceY = 0.0f;
	differenceX = 0.0f;
	quatZ = glm::angleAxis(0.0f, glm::vec3(0.0, 0.0, 1.0));

	return 	Core::createViewMatrixQuat(cameraPos, rotation);
}

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext context, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

unsigned int loadCubemap()
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << stbi_failure_reason() << std::endl;
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
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

void createSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}


void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);



	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.5f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 1.0f) * glm::mat4_cast(glm::inverse(rotation)) * shipInitialTransformation;
	drawObjectColor(shipContext, shipModelMatrix, glm::vec3(0.6f));

	drawObjectTexture(sphereContext, glm::translate(glm::vec3(0, 0, 0)), textureAsteroid);

	drawObjectTexture(terrainContext, glm::translate(glm::vec3(0, -2, 0)) * glm::scale(glm::vec3(1.0f)), textureTerrain);

	glUseProgram(programSkybox);
	glUniform1i(glGetUniformLocation(programSkybox, "skybox"), 0);
	glm::mat4 transformation = perspectiveMatrix * glm::mat4(glm::mat3(cameraMatrix));
	glUniformMatrix4fv(glGetUniformLocation(programSkybox, "projectionView"), 1, GL_FALSE, (float*)&transformation);
	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default

	// particles
	handleAllParticleSources(cameraPos, programParticles, cameraSide, cameraVertical, cameraMatrix, perspectiveMatrix);

	glUseProgram(0);
	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/skybox.vert","shaders/skybox.frag");
	programParticles = shaderLoader.CreateProgram("shaders/particles.vert", "shaders/particles.frag");

	loadModelToContext("models/spaceship.obj", shipContext);
	loadModelToContext("models/sphere.obj", sphereContext);
	loadModelToContext("models/ocean_floor3.obj", terrainContext);
	textureTerrain = Core::LoadTexture("textures/ocean_floor.jpg");
	textureAsteroid = Core::LoadTexture("textures/asteroid.png");
	cubemapTexture = loadCubemap();
	createSkybox();

	initParticles();
	addParticleSource(glm::vec3(3.45, -1.9, 1.45), 120.0f, 0.6f);
	addParticleSource(glm::vec3(-1.6, 1, 1.6), 180.0f, 0.3f);
	addParticleSource(glm::vec3(0, -2, 0), 100.0f, 1.5f);
	addParticleSource(glm::vec3(0, -2, -3), 100.0f, 1.5f);
	addParticleSource(glm::vec3(2, -2, -1), 100.0f, 1.5f);
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(programSkybox);
	shaderLoader.DeleteProgram(programParticles);

	shutdownParticles();
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
