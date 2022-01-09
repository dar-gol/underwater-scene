#pragma once
#include "glm.hpp"
#include "glew.h"
//#include "objload.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <iostream>

#define BUFFER_OFFSET(i) ((void*)(i))

namespace Core
{
	struct RenderContext {
		GLuint vertexArray;
		GLuint vertexBuffer;
		GLuint vertexIndexBuffer;
		unsigned int size = 0;

		void initFromAiMesh(aiMesh* mesh) {
			vertexArray = 0;
			vertexBuffer = 0;
			vertexIndexBuffer = 0;


            std::vector<float> textureCoord;
            std::vector<unsigned int> indices;
            //tex coord must be converted to 2d vecs
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                if (mesh->mTextureCoords[0] != nullptr) {
                    textureCoord.push_back(mesh->mTextureCoords[0][i].x);
                    textureCoord.push_back(mesh->mTextureCoords[0][i].y);
                }
                else {
                    textureCoord.push_back(0.0f);
                    textureCoord.push_back(0.0f);
                }
            }
            if (mesh->mTextureCoords[0] == nullptr) {
                std::cout << "no uv coords\n";
            }
            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                // retrieve all indices of the face and store them in the indices vector
                for (unsigned int j = 0; j < face.mNumIndices; j++)
                    indices.push_back(face.mIndices[j]);
            }

            unsigned int vertexDataBufferSize = sizeof(float) * mesh->mNumVertices * 3;
            unsigned int vertexNormalBufferSize = sizeof(float) * mesh->mNumVertices * 3;
            unsigned int vertexTexBufferSize = sizeof(float) * mesh->mNumVertices * 2;

		}

		~RenderContext() {
			glDeleteBuffers(1, &vertexBuffer);
			glDeleteBuffers(1, &vertexIndexBuffer);
			glDeleteVertexArrays(1, &vertexArray);
		}

	};

	void DrawContext(RenderContext& context);
}

