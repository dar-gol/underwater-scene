#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"
#include "Physics.h"
#include "Particles.h"
#include "Skybox.h"
#include "SOIL/stb_image_aug.h"

Core::Shader_Loader shaderLoader;
GLuint programColor, programTexture, programSkybox, programParticles;
GLuint boxTexture, groundTexture, cubemapTexture;

obj::Model planeModel, boxModel, sphereModel, cubeModel;
Core::RenderContext planeContext, boxContext, sphereContext, cubeContext;

float differenceX = 0.0f;
float differenceY = 0.0f;
float prevPosX = -1.0f;
float prevPosY = -1.0f;
glm::quat rotation = glm::quat(1, 0, 0, 0);
glm::quat quatZ;

glm::vec3 cameraPos = glm::vec3(0, 5, 20);
glm::vec3 cameraDir;
glm::vec3 cameraSide;
glm::vec3 cameraVertical;
glm::vec3 spherePos = cameraPos;
float cameraAngle = 0;
glm::mat4 cameraMatrix, perspectiveMatrix, cubeModelMatrix, controlActorModelMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(0.5, -1, -0.5));

// Initalization of physical scene (PhysX)
Physics pxScene(2 /* gravity (m/s^2) */);

const double physicsStepTime = 1.f / 60.f;
double physicsTimeToProcess = 0;

// physical objects
PxRigidStatic* planeBody = nullptr;
PxMaterial* planeMaterial = nullptr;
std::vector<PxRigidDynamic*> boxBodies;
PxMaterial* boxMaterial = nullptr;
PxRigidDynamic* sphereBody = nullptr;
PxMaterial* shipMaterial = nullptr;
PxRigidDynamic* skyboxBody = nullptr;

// renderable objects (description of a single renderable instance)
struct Renderable {
    Core::RenderContext* context;
    glm::mat4 modelMatrix;
    GLuint textureId;
};
std::vector<Renderable*> renderables;

// number of rows and columns of boxes the wall consists of
const int wallRows = 5;
const int wallCols = 5;

void initRenderables()
{
    // load models
    planeModel = obj::loadModelFromFile("models/plane.obj");
    boxModel = obj::loadModelFromFile("models/box.obj");
    sphereModel = obj::loadModelFromFile("models/spaceship.obj");
    cubeModel = obj::loadModelFromFile("models/cube.obj");

    planeContext.initFromOBJ(planeModel);
    boxContext.initFromOBJ(boxModel);
    sphereContext.initFromOBJ(sphereModel);
    cubeContext.initFromOBJ(cubeModel);

    // load textures
    groundTexture = Core::LoadTexture("textures/ocean_floor.jpg");
    boxTexture = Core::LoadTexture("textures/a.jpg");
    cubemapTexture = loadCubemap();

    // create ground
    Renderable* ground = new Renderable();
    ground->context = &planeContext;
    ground->textureId = groundTexture;
    renderables.emplace_back(ground);

    for (int i = 0; i < wallCols; i++)
        for (int j = 0; j < wallRows; j++) {
            // create box
            Renderable* box = new Renderable();
            box->context = &boxContext;
            box->textureId = boxTexture;
            renderables.emplace_back(box);
        }
    Renderable* sphere = new Renderable();
    sphere->context = &sphereContext;
    sphere->textureId = boxTexture;
    renderables.emplace_back(sphere);
}

void initPhysicsScene()
{
    planeBody = pxScene.physics->createRigidStatic(PxTransformFromPlaneEquation(PxPlane(0, 1, 0, 0)));
    planeMaterial = pxScene.physics->createMaterial(0.5, 0.5, 0.1);
    PxShape* planeShape = pxScene.physics->createShape(PxPlaneGeometry(), *planeMaterial);
    planeBody->attachShape(*planeShape);
    planeShape->release();
    planeBody->userData = renderables[0];
    pxScene.scene->addActor(*planeBody);

    //Wall
    boxMaterial = pxScene.physics->createMaterial(0.5, 0.5, 0.1);

    int next = 0;
    for (int i = 0; i < wallCols; i++)
        for (int j = 0; j < wallRows; j++) {
            boxBodies.push_back(pxScene.physics->createRigidDynamic(PxTransform(2.5 * i, 3 * j, 0)));
            PxShape* boxShape = pxScene.physics->createShape(PxBoxGeometry(1, 1, 1), *boxMaterial);
            boxBodies[next]->attachShape(*boxShape);
            boxShape->release();
            int x = i * wallRows + j + 1;
            boxBodies[next]->userData = renderables[x];
            //printf("x: %d\n", x);
            pxScene.scene->addActor(*boxBodies[next]);
            next++;
        }

    //Control actor
    spherePos = (cameraPos + cameraDir * 0.5f);

    sphereBody = pxScene.physics->createRigidDynamic(PxTransform(spherePos.x, spherePos.y, spherePos.z));
    //sphereBody->setLinearVelocity(PxVec3(0, 0, -30));
    shipMaterial = pxScene.physics->createMaterial(0.25, 0.5, 0.1);
    PxShape* sphereShape = pxScene.physics->createShape(PxBoxGeometry(1, 1, 1), *shipMaterial);
    sphereBody->attachShape(*sphereShape);
    sphereShape->release();
    sphereBody->userData = renderables[26];
    pxScene.scene->addActor(*sphereBody);
    sphereBody->setMaxAngularVelocity(PxReal(1));
    sphereBody->setMaxLinearVelocity(PxReal(20));
    sphereBody->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

    skyboxBody = pxScene.physics->createRigidDynamic(PxTransform(0, 0, 0));
    skyboxBody->userData = &cubeModelMatrix;
    pxScene.scene->addActor(*skyboxBody);
}

void updateTransforms()
{
    // Here we retrieve the current transforms of the objects from the physical simulation.
    auto actorFlags = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
    PxU32 nbActors = pxScene.scene->getNbActors(actorFlags);
    if (nbActors)
    {
        std::vector<PxRigidActor*> actors(nbActors);
        pxScene.scene->getActors(actorFlags, (PxActor**)&actors[0], nbActors);
        int index = 0;
        for (auto actor : actors)
        {
            // We use the userData of the objects to set up the model matrices
            // of proper renderables.
            if (!actor->userData) continue;
            Renderable* renderable = (Renderable*)actor->userData;
            // get world matrix of the object (actor)
            PxMat44 transform = actor->getGlobalPose();
            auto& c0 = transform.column0;
            auto& c1 = transform.column1;
            auto& c2 = transform.column2;
            auto& c3 = transform.column3;
            if (index == 26) {
                glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(0, -0.4f, 0)) * 
                    glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(1.5f));
                spherePos = glm::vec3(c3.x, c3.y, c3.z);
                renderable->modelMatrix = glm::mat4(
                    c0.x, c0.y, c0.z, c0.w,
                    c1.x, c1.y, c1.z, c1.w,
                    c2.x, c2.y, c2.z, c2.w,
                    c3.x, c3.y, c3.z, c3.w
                ) * shipInitialTransformation;

                controlActorModelMatrix = renderable->modelMatrix / shipInitialTransformation;
            }
            else {
                renderable->modelMatrix = glm::mat4(
                    c0.x, c0.y, c0.z, c0.w,
                    c1.x, c1.y, c1.z, c1.w,
                    c2.x, c2.y, c2.z, c2.w,
                    c3.x, c3.y, c3.z, c3.w
                );
            }


            index++;
        }
    }
}

void keyboard(unsigned char key, int x, int y)
{
    float angleSpeed = 20.0f;
    float moveSpeed = 50.0f;
    switch (key)
    {
    case 'z': cameraAngle -= angleSpeed; break;
    case 'x': cameraAngle += angleSpeed; break;
    case 'w': sphereBody->addForce(PxVec3(cameraDir.x * moveSpeed * 4, cameraDir.y * moveSpeed * 4, cameraDir.z * moveSpeed * 4)); break;
    case 's': sphereBody->addForce(PxVec3(0, 0, angleSpeed)); break;
    case 'd': sphereBody->addTorque(PxVec3(0, -angleSpeed, 0)); break;
    case 'a': sphereBody->addTorque(PxVec3(0, angleSpeed, 0)); break;
    case 'q': sphereBody->addForce(PxVec3(0, angleSpeed, 0)); break;
    case 'e': sphereBody->addForce(PxVec3(0, -angleSpeed, 0)); break;
    case 'i': sphereBody->addTorque(PxVec3(angleSpeed, 0, 0)); break;
    case 'k': sphereBody->addTorque(PxVec3(-angleSpeed, 0, 0)); break;
    case 'j': sphereBody->addTorque(PxVec3(0, 0, angleSpeed)); break;
    case 'l': sphereBody->addTorque(PxVec3(0, 0, -angleSpeed)); break;
    case 'r': 
        sphereBody->clearForce();
        sphereBody->clearTorque();
        break;
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

    auto rotX = glm::normalize(quatX);
    auto rotY = glm::normalize(quatY);

    rotation = glm::quat_cast((glm::translate(spherePos) * inverse(controlActorModelMatrix)));

    cameraDir = inverse(rotation) * glm::vec3(0, 0, -1);
    cameraSide = inverse(rotation) * glm::vec3(1, 0, 0);
    cameraVertical = inverse(rotation) * glm::vec3(0, 1, 0);

    differenceY = 0.0f;
    differenceX = 0.0f;

    cameraPos = glm::vec3(spherePos.x, spherePos.y, spherePos.z) + glm::mat3_cast(inverse(rotation)) * glm::vec3(0, 2, 7);

    return 	Core::createViewMatrixQuat(cameraPos, rotation);
}

void drawObjectColor(Core::RenderContext* context, glm::mat4 modelMatrix, glm::vec3 color)
{
    GLuint program = programColor;

    glUseProgram(program);

    glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

    Core::DrawContext(*context);

    glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext* context, glm::mat4 modelMatrix, GLuint textureId)
{
    GLuint program = programTexture;

    glUseProgram(program);

    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
    Core::SetActiveTexture(textureId, "textureSampler", program, 0);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

    Core::DrawContext(*context);

    glUseProgram(0);
}

void drawSkybox(GLuint program, Core::RenderContext* context, glm::mat4 modelMatrix, GLuint textureId)
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

void renderScene()
{
    double time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    static double prevTime = time;
    double dtime = time - prevTime;
    prevTime = time;

    // Update physics
    if (dtime < 1.f) {
        physicsTimeToProcess += dtime;
        while (physicsTimeToProcess > 0) {
            // here we perform the physics simulation step
            pxScene.step(physicsStepTime);
            physicsTimeToProcess -= physicsStepTime;
        }
    }

    // Update of camera and perspective matrices
    cameraMatrix = createCameraMatrix();
    perspectiveMatrix = Core::createPerspectiveMatrix();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

    // update transforms from physics simulation
    updateTransforms();

    for (int i = 0; i <= 26; i++) {
        drawObjectTexture(renderables[i]->context, renderables[i]->modelMatrix, renderables[i]->textureId);
    }

    //skybox
    drawSkybox(programSkybox, &cubeContext, cubeModelMatrix, cubemapTexture, cameraMatrix, lightDir, perspectiveMatrix);

    // particles NA KONCU
    handleAllParticleSources(cameraPos, programParticles, cameraSide, cameraVertical, cameraMatrix, perspectiveMatrix);

    glutSwapBuffers();
}

void init()
{
    srand(time(0));
    glEnable(GL_DEPTH_TEST);
    programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
    programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
    programSkybox = shaderLoader.CreateProgram("shaders/skybox.vert", "shaders/skybox.frag");
    programParticles = shaderLoader.CreateProgram("shaders/particles.vert", "shaders/particles.frag");

    initRenderables();
    initPhysicsScene();

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

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(600, 600);
    glutCreateWindow("OpenGL + PhysX");
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