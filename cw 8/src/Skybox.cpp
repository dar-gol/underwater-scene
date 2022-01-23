#include <iostream>
#include <cmath>
#include <vector>
#include "glew.h"
#include "SOIL/stb_image_aug.h"
#include "glm.hpp"
#include "Render_Utils.h"
#include "Texture.h"
#include "ext.hpp"

std::vector<std::string> faces{
    "textures/skybox/right.png",
    "textures/skybox/left.png",
    "textures/skybox/top.png",
    "textures/skybox/bottom.png",
    "textures/skybox/front.png",
    "textures/skybox/back.png",
};

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

void drawSkybox(GLuint program, 
    Core::RenderContext* context, 
    glm::mat4 modelMatrix, 
    GLuint textureId, 
    glm::mat4 cameraMatrix, 
    glm::vec3 lightDir,
    glm::mat4 perspectiveMatrix)
{
    glUseProgram(program);
    cameraMatrix = glm::mat4(glm::mat3(cameraMatrix));

    glDepthFunc(GL_LEQUAL);
    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
    Core::SetActiveTexture(textureId, "textureSampler", program, 0);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix * glm::scale(glm::vec3(200.0f));
    glUniformMatrix4fv(glGetUniformLocation(program, "projectionView"), 1, GL_FALSE, (float*)&transformation);

    Core::DrawContext(*context);

    glDepthFunc(GL_LESS);
    glUseProgram(0);
}