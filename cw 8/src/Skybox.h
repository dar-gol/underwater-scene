#pragma once
#include "glm.hpp"
#include "Render_Utils.h"

unsigned int loadCubemap();
void drawSkybox(GLuint program, 
	Core::RenderContext* context,
	glm::mat4 modelMatrix,
	GLuint textureId, 
	glm::mat4 cameraMatrix, 
	glm::vec3 lightDir, 
	glm::mat4 perspectiveMatrix);