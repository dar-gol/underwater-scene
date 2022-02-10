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

Core::RenderContext shipContext, terrainContext, seaweedContext, fishContext, seaweed2Context, aloeContext, fish2Context;

glm::vec3 cameraPos = glm::vec3(0, 5, 5);
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

GLuint textureSeaweed, textureTerrain, textureFish, textureSeaweed2, textureAloe, textureFish2;

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

float skyboxBoundary = 15.0f;

float skyboxSize = 10.0f;

std::vector<Core::Node> car;
std::vector<glm::vec3> keyPoints1({
	glm::vec3(200.0f, 2.0f, 29.0f),
	glm::vec3(195.0f, 2.0f, 29.0f),
	glm::vec3(190.0f, 8.0f, 29.0f),
	glm::vec3(185.0f, 8.0f, 29.0f),
	glm::vec3(180.0f, 8.0f, 29.0f),
	glm::vec3(175.0f, 8.0f, 29.0f),
	glm::vec3(170.0f, 8.0f, 29.0f),
	glm::vec3(165.0f, 8.0f, 29.0f),
	glm::vec3(160.0f, 8.0f, 29.0f),
	glm::vec3(155.0f, 8.0f, 29.0f),
	glm::vec3(150.0f, 8.0f, 29.0f),
	glm::vec3(145.0f, 8.0f, 29.0f),
	glm::vec3(140.0f, 8.0f, 29.0f),
	glm::vec3(135.0f, 8.0f, 29.0f),
	glm::vec3(130.0f, 8.0f, 29.0f),
	glm::vec3(124.0f, 8.0f, 29.0f),
	glm::vec3(120.0f,8.0f, 33.0f),
	glm::vec3(115.0f, 8.0f, 33.0f),
	glm::vec3(110.0f, 8.0f, 33.0f),
	glm::vec3(105.0f, 8.2f, 35.0f),
	glm::vec3(95.0f, 8.5f, 43.0f),
	glm::vec3(85.0f, 8.5f, 43.0f),
	glm::vec3(75.0f, 8.0f, 43.0f),
	glm::vec3(65.0f, 8.0f, 43.0f),
	glm::vec3(55.0f, 8.0f, 48.0f),
	glm::vec3(45.0f, 8.0f, 37.0f),
	glm::vec3(35.0f, 8.0f, 24.0f),
	glm::vec3(25.0f,8.0f, 18.0f),
	glm::vec3(15.0f, 8.0f, 4.0f),
	glm::vec3(5.0f, 8.0f, -6.0f),
	glm::vec3(-5.0f, 8.0f, -16.0f),
	glm::vec3(-15.0f, 8.0f, -26.0f),
	glm::vec3(-16.0f, 8.0f, -30.0f),
	glm::vec3(-18.0f, 8.0f, -40.0f),
	glm::vec3(-16.0f, 8.0f, -50.0f),
	glm::vec3(-10.0f, 8.0f, -60.0f),
	glm::vec3(-12.0f, 8.0f, -70.0f),
	glm::vec3(-13.0f, 8.0f, -80.0f),
	glm::vec3(-12.0f, 8.0f, -90.0f)
	});
std::vector<glm::quat> keyRotation1;

std::vector<glm::vec3> keyPoints2({
	glm::vec3(-12.0f, 20.0f, 300.0f),
	glm::vec3(-13.0f, 19.0f, 80.0f),
	glm::vec3(-12.0f, 18.0f, 70.0f),
	glm::vec3(-10.0f, 17.0f, 60.0f),
	glm::vec3(-16.0f, 16.0f, 50.0f),
	glm::vec3(-18.0f, 15.0f, 40.0f),
	glm::vec3(-16.0f, 14.0f, 30.0f),
	glm::vec3(-15.0f, 13.0f, 26.0f),
	glm::vec3(-5.0f, 8.0f, 16.0f),
	glm::vec3(5.0f, 8.0f, 6.0f),
	glm::vec3(15.0f, 8.0f, -4.0f),
	glm::vec3(25.0f,8.0f, -18.0f),
	glm::vec3(35.0f, 8.0f, -24.0f),
	glm::vec3(45.0f, 8.0f, -37.0f),
	glm::vec3(55.0f, 8.0f, -48.0f),
	glm::vec3(65.0f, 8.0f, -43.0f),
	glm::vec3(75.0f, 8.0f, -43.0f),
	glm::vec3(85.0f, 8.5f, -43.0f),
	glm::vec3(95.0f, 8.5f, -43.0f),
	glm::vec3(105.0f, 8.2f, -35.0f),
	glm::vec3(110.0f, 8.0f, -33.0f),
	glm::vec3(115.0f, 8.0f, -33.0f),
	glm::vec3(120.0f,8.0f, -33.0f),
	glm::vec3(124.0f, 8.0f, -29.0f),
	glm::vec3(130.0f, 8.0f, -29.0f),
	glm::vec3(135.0f, 8.0f, -29.0f),
	glm::vec3(140.0f, 8.0f, -29.0f),
	glm::vec3(145.0f, 8.0f, -29.0f),
	glm::vec3(150.0f, 8.0f, -29.0f),
	glm::vec3(155.0f, 8.0f, -29.0f),
	glm::vec3(160.0f, 8.0f, -29.0f),
	glm::vec3(165.0f, 8.0f, -29.0f),
	glm::vec3(170.0f, 8.0f, -29.0f),
	glm::vec3(175.0f, 8.0f, -29.0f),
	glm::vec3(180.0f, 8.0f, -29.0f),
	glm::vec3(185.0f, 8.0f, -29.0f),
	glm::vec3(190.0f, 8.0f, -29.0f),
	glm::vec3(195.0f, 2.0f, -29.0f),
	glm::vec3(200.0f, 2.0f, -29.0f),
});
std::vector<glm::quat> keyRotation2;

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

bool isInBoundaries(glm::vec3 nextPosition) {
	return nextPosition.z > -skyboxBoundary && nextPosition.z < skyboxBoundary 
		&& nextPosition.y > 5.0f && nextPosition.y < skyboxBoundary
		&& nextPosition.x < skyboxBoundary && nextPosition.x > -skyboxBoundary;
}

glm::mat4 animationMatrix(float time, std::vector<glm::vec3> keyPoints, std::vector<glm::quat> keyRotation) {
	float speed = 2.;
	time = time * speed;
	std::vector<float> distances;
	float timeStep = 0;
	for (int i = 0; i < keyPoints.size() - 1; i++) {
		timeStep += (keyPoints[i] - keyPoints[i + 1]).length();
		distances.push_back((keyPoints[i] - keyPoints[i + 1]).length());
	}
	time = fmod(time, timeStep);

	int index = 0;

	while (distances[index] <= time) {
		time = time - distances[index];
		index += 1;
	}

	float t = time / distances[index];

	int size = keyPoints.size() - 1;
	int rotationSize = keyRotation.size() - 1;

	glm::vec3 pos = glm::catmullRom(keyPoints[index - 1], keyPoints[index], keyPoints[index + 1], keyPoints[index + 2], t);

	auto a1 = keyRotation[index] * glm::exp(-(glm::log(glm::inverse(keyRotation[index]) * keyRotation[index - 1]) + glm::log(glm::inverse(keyRotation[index]) * keyRotation[index + 1])) * glm::quat(1 / 4, 1 / 4, 1 / 4, 1 / 4));
	auto a2 = keyRotation[index + 1] * glm::exp(-(glm::log(glm::inverse(keyRotation[index + 1]) * keyRotation[index]) + glm::log(glm::inverse(keyRotation[index + 1]) * keyRotation[index + 2])) * glm::quat(1 / 4, 1 / 4, 1 / 4, 1 / 4));
	auto animationRotation = glm::squad(keyRotation[index], keyRotation[index + 1], a1, a2, t);

	glm::mat4 result = glm::translate(pos) * glm::mat4_cast(animationRotation);

	return result;
}

void keyboard(unsigned char key, int x, int y)
{

	float angleSpeed = 0.01f;
	float moveSpeed = 0.1f;
	glm::vec3 nextPosition;
	switch (key)
	{
		case 'z': quatZ = glm::angleAxis(angleSpeed, glm::vec3(0.0, 0.0, 1.0)); break;
		case 'x': quatZ = glm::angleAxis(-angleSpeed, glm::vec3(0.0, 0.0, 1.0)); break;
		case 'w': nextPosition = cameraPos + cameraDir * moveSpeed; break;
		case 's': nextPosition = cameraPos - cameraDir * moveSpeed; break;
		case 'd': nextPosition = cameraPos + cameraSide * moveSpeed; break;
		case 'a': nextPosition = cameraPos - cameraSide * moveSpeed; break;
		case 'q': nextPosition = cameraPos + cameraVertical * moveSpeed; break;
		case 'e': nextPosition = cameraPos - cameraVertical * moveSpeed; break;
	}
	if (isInBoundaries(nextPosition)) {
		cameraPos = nextPosition;
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
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.f;
	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

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

	glClear(GL_DEPTH_BUFFER_BIT);

	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.5f, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 1.0f) * glm::mat4_cast(glm::inverse(rotation)) * shipInitialTransformation;
	drawObjectColor(shipContext, shipModelMatrix, glm::vec3(0.6f));

	drawObjectTexture(terrainContext, glm::translate(glm::vec3(0, -2, 0)) * glm::scale(glm::vec3(1.0f)), textureTerrain);

	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(1, 2.5f, -4)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(1, 3, -2)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(1, 1.5f, 2)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(-3, 3.5f, 10)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(-5, 4.5f, 10)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(-8, 5.5f, 10)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(-10, 6.5f, 8)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(-12, 6.5f, 8)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));
	drawObjectColor(seaweed2Context, glm::translate(glm::vec3(-15, 6.5f, 8)) * glm::scale(glm::vec3(1.5f)), glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-8, 5, 6)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-13, 5.3f, 3)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-13, 5.3f, 3)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-15, 6.5f, 7)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-18, 7.0f, -12)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-16, 7.0f, -10)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-0, 1, 6)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-2, 2.0f, 3)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(2, 2.0f, 3)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-4, 2.5f, 7)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-6, 2.0f, -12)) * glm::scale(glm::vec3(0.5f)), textureAloe);
	drawObjectTexture(aloeContext, glm::translate(glm::vec3(-1, 2.0f, -10)) * glm::scale(glm::vec3(0.5f)), textureAloe);


	drawObjectTexture(seaweedContext, glm::translate(glm::vec3(-9, 5.8f, -15)) * glm::scale(glm::vec3(1.0f)), textureSeaweed);
	drawObjectTexture(seaweedContext, glm::translate(glm::vec3(-11, 5.8f, -15)) * glm::scale(glm::vec3(1.0f)), textureSeaweed);
	drawObjectTexture(seaweedContext, glm::translate(glm::vec3(-9, 5, -13)) * glm::scale(glm::vec3(1.0f)), textureSeaweed);
	drawObjectTexture(seaweedContext, glm::translate(glm::vec3(-12, 5.8f, -13)) * glm::scale(glm::vec3(1.0f)), textureSeaweed);
	drawObjectTexture(seaweedContext, glm::translate(glm::vec3(-16, 7.1f, -13)) * glm::scale(glm::vec3(1.0f)), textureSeaweed);
	drawObjectTexture(seaweedContext, glm::translate(glm::vec3(-15, 7.1f, -12)) * glm::scale(glm::vec3(1.0f)), textureSeaweed);

	glDepthFunc(GL_LESS); // set depth function back to default

	// particles
	handleAllParticleSources(cameraPos, programParticles, cameraSide, cameraVertical, cameraMatrix, perspectiveMatrix);
	glm::vec3  transform1 = glm::vec3(0, 3, 0);
	glm::vec3  transform2 = glm::vec3(0, 0, 3);
	glm::vec3  transform3 = glm::vec3(3, 0, 0);
	glm::vec3  transform4 = glm::vec3(0, 2, 1);
	for (int i = 0; i < 30; i++) {
		if (time > -10) {
			glm::mat4 matrix = animationMatrix(time + 15, keyPoints1, keyRotation1);
			drawObjectTexture(fishContext, matrix * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(transform1) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(transform2) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(transform3) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(transform4) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(1, 3, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(0.5f, 0.5f, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(0, 1, 0.5f)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(0.8f, 1.2f, 0.5f)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(1, 3, 1)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(0.3f, 2, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(0.1f, 1, 0.1f)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			drawObjectTexture(fishContext, matrix * glm::translate(glm::vec3(0.6f, 2, 0.6f)) * glm::rotate(glm::radians(0.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(2.5f)), textureFish);
			
			matrix = animationMatrix(time + 15, keyPoints2, keyRotation2);
			drawObjectTexture(fish2Context, matrix * glm::rotate(glm::radians(180.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.5f)), textureFish2);
			drawObjectTexture(fish2Context, matrix * glm::translate(transform1) * glm::rotate(glm::radians(180.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.5f)), textureFish2);
			drawObjectTexture(fish2Context, matrix * glm::translate(transform2) * glm::rotate(glm::radians(180.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.5f)), textureFish2);
			drawObjectTexture(fish2Context, matrix * glm::translate(transform3) * glm::rotate(glm::radians(180.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.5f)), textureFish2);
			drawObjectTexture(fish2Context, matrix * glm::translate(transform4) * glm::rotate(glm::radians(180.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::radians(0.0f), glm::vec3(0, 0, 1)) * glm::rotate(glm::radians(270.0f), glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(0.5f)), textureFish2);
			
			time -= 3;
		}
	}
	glUseProgram(0);
	glutSwapBuffers();
}

void initKeyRoation(std::vector<glm::vec3>& keyPoints, std::vector<glm::quat>& keyRotation) {
	glm::vec3 oldDirection = glm::vec3(0, 0, 1);
	glm::quat oldRotationCamera = glm::quat(1, 0, 0, 0);
	for (int i = 0; i < keyPoints.size() - 1; i++) {
		glm::vec3 direction = glm::normalize(keyPoints[i + 1] - keyPoints[i]);
		glm::quat rotation = normalize(glm::rotationCamera(oldDirection, direction) * oldRotationCamera);
		keyRotation.push_back(rotation);
		oldDirection = direction;
		oldRotationCamera = rotation;
	}
	keyRotation.push_back(glm::quat(1, 0, 0, 0));
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
	loadModelToContext("models/seaweed.obj", seaweedContext);
	loadModelToContext("models/seaweed2.obj", seaweed2Context);
	loadModelToContext("models/aloe.obj", aloeContext);
	loadModelToContext("models/fish1.obj", fishContext);
	loadModelToContext("models/ocean_floor3.obj", terrainContext);
	loadModelToContext("models/fish2.obj", fish2Context);
	textureTerrain = Core::LoadTexture("textures/ocean_floor.jpg");
	textureSeaweed = Core::LoadTexture("textures/seaweed.png");
	textureAloe = Core::LoadTexture("textures/aloe.png");
	textureFish = Core::LoadTexture("textures/fish1.png");
	textureFish2 = Core::LoadTexture("textures/fish2.png");
	cubemapTexture = loadCubemap();
	createSkybox();

	initParticles();
	addParticleSource(glm::vec3(3.45, -1.9, 1.45), 120.0f, 0.6f);
	addParticleSource(glm::vec3(-1.6, 1, 1.6), 180.0f, 0.3f);
	addParticleSource(glm::vec3(0, -2, 0), 100.0f, 1.5f);
	addParticleSource(glm::vec3(0, -2, -3), 100.0f, 1.5f);
	addParticleSource(glm::vec3(2, -2, -1), 100.0f, 1.5f);

	initKeyRoation(keyPoints1, keyRotation1);
	initKeyRoation(keyPoints2, keyRotation2);
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
