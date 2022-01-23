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

Core::Shader_Loader shaderLoader;
GLuint programColor;
GLuint programTexture;

obj::Model planeModel;
obj::Model boxModel;
glm::mat4 boxModelMatrix;
GLuint boxTexture, groundTexture;
Core::RenderContext planeContext, boxContext;

glm::vec3 cameraPos = glm::vec3(0, 5, 20);
glm::vec3 cameraDir;
glm::vec3 cameraSide;
float cameraAngle = 0;
glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(0.5, -1, -0.5));


// Initalization of physical scene (PhysX)
Physics pxScene(9.8 /* gravity (m/s^2) */);

// fixed timestep for stable and deterministic simulation
const double physicsStepTime = 1.f / 60.f;
double physicsTimeToProcess = 0;

// physical objects
PxRigidStatic *planeBody = nullptr;
PxMaterial *planeMaterial = nullptr;
PxRigidDynamic *boxBody = nullptr;
PxMaterial *boxMaterial = nullptr;


void initRenderables()
{
    // load models
    planeModel = obj::loadModelFromFile("models/plane.obj");
    boxModel = obj::loadModelFromFile("models/box.obj");

    // load textures
    groundTexture = Core::LoadTexture("textures/sand.jpg");
    boxTexture = Core::LoadTexture("textures/a.jpg");

    planeContext.initFromOBJ(planeModel);
    boxContext.initFromOBJ(boxModel);
}

void initPhysicsScene()
{
    //-----------------------------------------------------------
    // DOCS
    //-----------------------------------------------------------
    //https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxguide/Manual/Index.html
    //https://gameworksdocs.nvidia.com/PhysX/4.1/documentation/physxapi/files/index.html


    //-----------------------------------------------------------
    // TASKS
    //-----------------------------------------------------------
    // IMPORTANT:
    //   * to create objects use:     pxScene.physics
    //   * to manage the scene use:   pxScene.scene
    //
    // Your task is to create two physical objects: one static body
    // and one dynamic body.
    // These will be a planar ground and a box.
    // If everything is set up correctly, the box should fall down,
    // bounce a few times and finally rest on the ground.
    //
    // You should provide the physics initialization code below.
    // The required global variables have already been declared.
    // Note, that most of the variables are pointers, so remember
    // to use -> instead of . to access member methods/variables


    //**************************
    // I. Create plane:
    
    // 1. Create plane body.
    // Use method: createRigidStatic( transform )
    // As the argument provide the transform created from the plane equation (n=[0,1,0], d=0):
    //      PxTransformFromPlaneEquation(PxPlane(0, 1, 0, 0))
    // Save the created body in global variable: planeBody
    
    // 2. Create material.
    // Use method: createMaterial( staticFriction, dynamicFriction, restitution )
    // Use parameters e.g. 0.5, 0.5, 0.6
    // Save the material in global variable: planeMaterial

    // 3. Create plane shape.
    // Use method: createShape( geometry, material )
    // As the first argument use plane geometry: simply PxPlaneGeometry()
    // As the second argument use the just created plane material.
    // Save the shape in local variable: PxShape* planeShape

    // 4. Attach shape to the body.
    // Use method attachShape( shape ) on planeBody.
    // As the argument provide the just created plane shape.

    // 5. Now the shape is attached, it's no longer needed, so release it.
    // Use method release on planeShape.

    // 6. You can also provide a custom data for the body
    // It will be useful for the box, to bind the physical body
    // with its graphical representation. As far as the plane is concerned,
    // just set the user data to null, since the plane is static anyway.
    // Set the member variable userData of planeBody to nullptr.

    // 7. Finally, add the body to the scene.
    // Use method: addActor( body )
    // As the argument provide the plane body.
    
    //**************************
    // II. Create box:
    
    // 1. Create box body.
    // Use method: createRigidDynamic( transform )
    // Set the initial transform of the box to 5 meters above the ground.
    // Use: PxTransform(x, y, z)
    // Save the created body in global variable: boxBody
    
    // 2. Create material.
    // Use method: createMaterial( staticFriction, dynamicFriction, restitution )
    // Use parameters e.g. 0.5, 0.5, 0.6
    // Save the material in global variable: boxMaterial

    // 3. Create box shape.
    // Use method: createShape( geometry, material )
    // As the first argument provide box geometry.
    // Use: PxBoxGeometry(hx, hy, hz), where the arguments define
    // half extents of the box (use hx=hy=hz=1).
    // As the second argument use the just created box material.
    // Save the shape in local variable: PxShape* boxShape


    // 4. Attach shape to the body.
    // Use method attachShape( shape ) on boxBody.
    // As the argument provide the just created box shape.

    // 5. Now the shape is attached, it's no longer needed, so release it.
    // Use method release on boxShape.

    // 6. In order to render the box corresponding to its physical state,
    // we need to bind the model matrix used for drawing to the transform
    // of the body in the physics simulation.
    // Set the member variable userData of boxBody to the pointer of boxModelMatrix.
    // (userData is of universal type void*, so be careful when
    // accessing it later - remember to cast it back to the correct type)

    // 7. Finally, add the body to the scene.
    // Use method: addActor( body )
    // As the argument provide the box body.

    //**************************
    // III. Now read the function updateTransforms()
    // See how we use the userData of the bodies to properly update
    // the transforms of their visualizations.

    //**************************
    // IV. See also the function renderScene()
    // Notice how the physics simulation is updated and the objects are rendered.
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
        for (auto actor : actors)
        {
            // We use the userData of the objects to set up the proper model matrices.
            if (!actor->userData) continue;
            glm::mat4 *modelMatrix = (glm::mat4*)actor->userData;

            // get world matrix of the object (actor)
            PxMat44 transform = actor->getGlobalPose();
            auto &c0 = transform.column0;
            auto &c1 = transform.column1;
            auto &c2 = transform.column2;
            auto &c3 = transform.column3;

            // set up the model matrix used for the rendering
            *modelMatrix = glm::mat4(
                c0.x, c0.y, c0.z, c0.w,
                c1.x, c1.y, c1.z, c1.w,
                c2.x, c2.y, c2.z, c2.w,
                c3.x, c3.y, c3.z, c3.w);
        }
    }
}

void keyboard(unsigned char key, int x, int y)
{
    float angleSpeed = 0.1f;
    float moveSpeed = 0.1f;
    switch (key)
    {
    case 'z': cameraAngle -= angleSpeed; break;
    case 'x': cameraAngle += angleSpeed; break;
    case 'w': cameraPos += cameraDir * moveSpeed; break;
    case 's': cameraPos -= cameraDir * moveSpeed; break;
    case 'd': cameraPos += cameraSide * moveSpeed; break;
    case 'a': cameraPos -= cameraSide * moveSpeed; break;
    }
}

void mouse(int x, int y)
{
}

glm::mat4 createCameraMatrix()
{
    cameraDir = glm::normalize(glm::vec3(cosf(cameraAngle - glm::radians(90.0f)), 0, sinf(cameraAngle - glm::radians(90.0f))));
    glm::vec3 up = glm::vec3(0, 1, 0);
    cameraSide = glm::cross(cameraDir, up);

    return Core::createViewMatrix(cameraPos, cameraDir, up);
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

    // render models
    drawObjectTexture(planeContext, glm::rotate(glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f)), groundTexture);
    drawObjectTexture(boxContext, boxModelMatrix, boxTexture); // boxModelMatrix was updated in updateTransforms()

    glutSwapBuffers();
}

void init()
{
    srand(time(0));
    glEnable(GL_DEPTH_TEST);
    programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
    programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");

    initRenderables();
    initPhysicsScene();
}

void shutdown()
{
    shaderLoader.DeleteProgram(programColor);
    shaderLoader.DeleteProgram(programTexture);

    planeContext.initFromOBJ(planeModel);
    boxContext.initFromOBJ(boxModel);
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
