#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>


GLuint program;
Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;


float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;


void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'a': cameraAngle -= angleSpeed; break;
	case 'd': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	}
}

glm::mat4 createCameraMatrix()
{
	cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0,1,0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	glUseProgram(program);

	glm::mat4 camera = createCameraMatrix();
	glm::mat4 perspective = Core::createPerspectiveMatrix();
	glm::mat4 transformation = perspective * camera;

	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);

	glUseProgram(0);
	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_2_2.vert", "shaders/shader_2_2.frag");
}

void shutdown()
{
	shaderLoader.DeleteProgram(program);
}

void loadModelToContext(std::string path,Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
}


void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	loadModelToContext(".\\models\\spaceship.obj", sphereContext);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
