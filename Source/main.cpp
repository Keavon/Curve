#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <glad/glad.h>
#include <cmath>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <glad/glad.h>
#include <iomanip>
#include <iostream>
#include <string>
#include <irrKlang.h>

#include "Camera.h"
#include "GLSL.h"
#include "GLTextureWriter.h"
#include "MatrixStack.h"
#include "Object3D.h"
#include "Program.h"
#include "Shape.h"
#include "Skybox.h"
#include "WindowManager.h"
#include "engine/Collider.h"
#include "engine/ColliderSphere.h"
#include "engine/GameObject.h"
#include "gameobjects/Ball.h"
#include "gameobjects/Box.h"
#include "gameobjects/Goal.h"
#include "gameobjects/Enemy.h"
#include "engine/ColliderSphere.h"
#include "engine/Collider.h"
#include "engine/Octree.h"
#include "engine/Frustum.h"

// value_ptr for glm
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// number of skin textures to load and swap through
#define NUMBER_OF_MARBLE_SKINS 13
#define SHADOW_QUALITY 1 // [-1, 0, 1, 2, 3, 4] (-1: default) (0: OFF);

using namespace std;
using namespace glm;

extern bool ballInGoal;

class Application : public EventCallbacks {

public:
    WindowManager *windowManager = nullptr;

    // Game Info Globals
    float START_TIME = 0.0f;
    bool DID_WIN = false;
    bool MOVING = false;
    bool MOUSE_DOWN = false;
    bool SHOW_CURSOR = false;
    int SCORE = 0;
    int CURRENT_SKIN = 0;
    vec3 START_POSITION = vec3(120, 3, 7);
	vec3 CENTER_LVL_POSITION = vec3(70, 3, 40);

    // Shadow Globals
    int SHADOWS = 1;
    int SHADOW_AA = 4;
    int DEBUG_LIGHT = 0;
    int GEOM_DEBUG = 1;
    GLuint SHADOW_SIZE = 0;
    GLuint depthMapFBO = 0;
    GLuint depthMap = 0;

    // Light Position Globals
    vec3 gameLight = vec3(300, 150, 250);
    vec3 gameLightColor = vec3(250000, 250000, 250000);

    // Shader programs
    shared_ptr<Program> texProg;
    shared_ptr<Program> skyProg;
    shared_ptr<Program> circleProg;
    shared_ptr<Program> cubeOutlineProg;
    shared_ptr<Program> DepthProg;
    shared_ptr<Program> DepthProgDebug;
    shared_ptr<Program> DebugProg;

    // Shapes
    shared_ptr<Shape> cube;
    shared_ptr<Shape> roboHead;
    shared_ptr<Shape> roboLeg;
    shared_ptr<Shape> roboFoot;
    shared_ptr<Shape> boxModel;
    shared_ptr<Shape> plane;
    shared_ptr<Shape> billboard;
    shared_ptr<Shape> goalModel;
    shared_ptr<Shape> sphere;

    // Camera
    shared_ptr<Camera> camera;
    Frustum viewFrustum;

    // Game objects
    shared_ptr<Ball> ball;
    shared_ptr<Enemy> enemy;
    shared_ptr<Enemy> enemy2;
    shared_ptr<Goal> goal;
    shared_ptr<Box> goalObject;
    vector<shared_ptr<PhysicsObject>> boxes;
    shared_ptr<Octree> octree;

    // BillBoard for rendering a texture to screen. (like the shadow map)
    GLuint quad_VertexArrayID;
    GLuint quad_vertexbuffer;

    // Textures
    shared_ptr<Skybox> skyboxTexture;

    shared_ptr<Texture> crateAlbedo;
    shared_ptr<Texture> crateRoughness;
    shared_ptr<Texture> crateMetallic;

    shared_ptr<Texture> panelAlbedo;
    shared_ptr<Texture> panelRoughness;
    shared_ptr<Texture> panelMetallic;

    vector<shared_ptr<Texture>> marbleTextures;

    void init(const string &resourceDirectory) {
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        GLSL::checkVersion();

        // Set background color.
        glClearColor(.12f, .34f, .56f, 1.0f);
        // Enable z-buffer test.
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        // Initialize camera
        camera = make_shared<Camera>(windowManager, CENTER_LVL_POSITION);
        camera->init();

        ballInGoal = false;

        
    }

    //=================================================
    // SHADERS
    void initShaders(const string &resourceDirectory) {
        initShader_skyProg(resourceDirectory);
        initShader_texProg(resourceDirectory);

        initShader_circleProg(resourceDirectory);
        initShader_cubeOutlineProg(resourceDirectory);

        initShader_DepthProg(resourceDirectory);
        initShader_DebugProg(resourceDirectory);
        initShader_DepthProgDebug(resourceDirectory);
    }

    void initShader_DepthProg(const string &resourceDirectory) {
        DepthProg = make_shared<Program>();
        DepthProg->setVerbose(true);
        DepthProg->setShaderNames(resourceDirectory + "/shaders/depth_vert.glsl",
            resourceDirectory + "/shaders/depth_frag.glsl");
        if (!DepthProg->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!"
                << std::endl;
            exit(1);
        }

        DepthProg->addUniform("LP");
        DepthProg->addUniform("LV");
        DepthProg->addUniform("M");
        DepthProg->addAttribute("vertPos");
        // un-needed, but easier then modifying shape
        DepthProg->addAttribute("vertNor");
        DepthProg->addAttribute("vertTex");
    }

    void initShader_DepthProgDebug(const string &resourceDirectory) {
        DepthProgDebug = make_shared<Program>();
        DepthProgDebug->setVerbose(true);
        DepthProgDebug->setShaderNames(
            resourceDirectory + "/shaders/depth_vertDebug.glsl",
            resourceDirectory + "/shaders/depth_fragDebug.glsl");
        if (!DepthProgDebug->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!"
                << std::endl;
            exit(1);
        }

        DepthProgDebug->addUniform("LP");
        DepthProgDebug->addUniform("LV");
        DepthProgDebug->addUniform("M");
        DepthProgDebug->addAttribute("vertPos");
        // un-needed, but easier then modifying shape
        DepthProgDebug->addAttribute("vertNor");
        DepthProgDebug->addAttribute("vertTex");
    }

    void initShader_DebugProg(const string &resourceDirectory) {
        DebugProg = make_shared<Program>();
        DebugProg->setVerbose(true);
        DebugProg->setShaderNames(resourceDirectory + "/shaders/pass_vert.glsl",
            resourceDirectory + "/shaders/pass_texfrag.glsl");
        if (!DebugProg->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!"
                << std::endl;
            exit(1);
        }

        DebugProg->addUniform("texBuf");
        DebugProg->addAttribute("vertPos");
    }

    void initShader_texProg(const string &resourceDirectory) {
        // Shader for textured models
        texProg = make_shared<Program>();
        texProg->setVerbose(true);
        texProg->setShaderNames(resourceDirectory + "/shaders/pbr.vert.glsl",
            resourceDirectory + "/shaders/pbr.frag.glsl");
        if (!texProg->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!"
                << std::endl;
            exit(1);
        }

        texProg->addAttribute("vertPos");
        texProg->addAttribute("vertNor");
        texProg->addAttribute("vertTex");

        texProg->addUniform("P");
        texProg->addUniform("V");
        texProg->addUniform("M");

        texProg->addUniform("shadows");
        texProg->addUniform("shadowSize");
        texProg->addUniform("shadowAA");
        texProg->addUniform("shadowDepth");
        texProg->addUniform("LS");

        texProg->addUniform("albedoMap");
        texProg->addUniform("roughnessMap");
        texProg->addUniform("metallicMap");

        texProg->addUniform("roughness");
        texProg->addUniform("metallic");

        texProg->addUniform("lightPosition");
        texProg->addUniform("lightColor");

        texProg->addUniform("viewPos");
    }

    void initShader_skyProg(const string &resourceDirectory) {
        // Shader for skybox
        skyProg = make_shared<Program>();
        skyProg->setVerbose(true);
        skyProg->setShaderNames(
            resourceDirectory + "/shaders/sky_vert.glsl",
            resourceDirectory + "/shaders/sky_frag.glsl");
        if (!skyProg->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        skyProg->addUniform("P");
        skyProg->addUniform("V");
        skyProg->addUniform("Texture0");
        skyProg->addAttribute("vertPos");
    }

    void initShader_circleProg(const string &resourceDirectory) {
        // Shader for debug circle
        circleProg = make_shared<Program>();
        circleProg->setVerbose(true);
        circleProg->setShaderNames(
            resourceDirectory + "/shaders/circle_vert.glsl",
            resourceDirectory + "/shaders/circle_frag.glsl");
        if (!circleProg->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        circleProg->addUniform("P");
        circleProg->addUniform("V");
        circleProg->addUniform("M");
        circleProg->addUniform("radius");
        circleProg->addAttribute("vertPos");
    }

    void initShader_cubeOutlineProg(const string &resourceDirectory) {
        // Shader for debug cube
        cubeOutlineProg = make_shared<Program>();
        cubeOutlineProg->setVerbose(true);
        cubeOutlineProg->setShaderNames(
            resourceDirectory + "/shaders/cube_outline_vert.glsl",
            resourceDirectory + "/shaders/cube_outline_frag.glsl");
        if (!cubeOutlineProg->init()) {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        cubeOutlineProg->addUniform("P");
        cubeOutlineProg->addUniform("V");
        cubeOutlineProg->addUniform("M");
        cubeOutlineProg->addUniform("edge");
        cubeOutlineProg->addAttribute("vertPos");
    }
    //=================================================
    //=================================================
    // TEXTURES
    void initTextures(const string &resourceDirectory) {
        initCrateAlbedo(resourceDirectory);
        initCrateRoughness(resourceDirectory);
        initCrateMetallic(resourceDirectory);

        initPanelAlbedo(resourceDirectory);
        initPanelRoughness(resourceDirectory);
        initPanelMetallic(resourceDirectory);

        initMarbleTexture(resourceDirectory);
        initSkyBox(resourceDirectory);
        initShadow();
    }

    void initCrateAlbedo(const string &resourceDirectory) {
        crateAlbedo = make_shared<Texture>();
        crateAlbedo->setFilename(resourceDirectory + "/textures/pbr/pbr_albedo_metal.png");
        crateAlbedo->init();
        crateAlbedo->setUnit(1);
        crateAlbedo->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

    void initCrateRoughness(const string &resourceDirectory) {
        crateRoughness = make_shared<Texture>();
        crateRoughness->setFilename(resourceDirectory + "/textures/pbr/pbr_roughness_metal.png");
        crateRoughness->init();
        crateRoughness->setUnit(2);
        crateRoughness->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

    void initCrateMetallic(const string &resourceDirectory) {
        crateMetallic = make_shared<Texture>();
        crateMetallic->setFilename(resourceDirectory + "/textures/pbr/pbr_metallic_metal.png");
        crateMetallic->init();
        crateMetallic->setUnit(3);
        crateMetallic->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

    void initPanelAlbedo(const string &resourceDirectory) {
        panelAlbedo = make_shared<Texture>();
        panelAlbedo->setFilename(resourceDirectory + "/textures/pbr/pbr_panel_albedo.jpg");
        panelAlbedo->init();
        panelAlbedo->setUnit(4);
        panelAlbedo->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

    void initPanelRoughness(const string &resourceDirectory) {
        panelRoughness = make_shared<Texture>();
        panelRoughness->setFilename(resourceDirectory + "/textures/pbr/pbr_panel_roughness.jpg");
        panelRoughness->init();
        panelRoughness->setUnit(5);
        panelRoughness->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

    void initPanelMetallic(const string &resourceDirectory) {
        panelMetallic = make_shared<Texture>();
        panelMetallic->setFilename(resourceDirectory + "/textures/pbr/pbr_panel_metallic.jpg");
        panelMetallic->init();
        panelMetallic->setUnit(6);
        panelMetallic->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }


    void initMarbleTexture(const string &resourceDirectory) {
        // loops over the number of skin textures, initializing them and adding them to a vector
        string textureBaseFolder, textureNumber, textureExtension, textureName;
        double completion = 0;
        for (int i = 0; i < NUMBER_OF_MARBLE_SKINS; i++) {
            textureBaseFolder = "/textures/marble/albedo/";
            textureNumber = to_string(i);
            textureExtension = ".jpg";

            textureName = textureBaseFolder + textureNumber + textureExtension;
            completion = ((float)i * 100 / (float)NUMBER_OF_MARBLE_SKINS);

            cout << std::setprecision(2) << "Loading Textures: " << completion << "\% complete." << endl;

            shared_ptr<Texture> marbleTexture = make_shared<Texture>();
            marbleTexture->setFilename(resourceDirectory + textureName);
            marbleTexture->init();
            marbleTexture->setUnit(1);
            marbleTexture->setWrapModes(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);

            marbleTextures.push_back(marbleTexture);
        }
        cout << "Loading Textures: complete." << endl;
    }

    void initSkyBox(const string &resourceDirectory) {
        // Load skybox
        string skyboxFilenames[] = { "sea_ft.JPG", "sea_bk.JPG", "sea_up.JPG",
                                    "sea_dn.JPG", "sea_rt.JPG", "sea_lf.JPG" };
        for (int i = 0; i < 6; i++) {
            skyboxFilenames[i] = resourceDirectory + "/skybox/" + skyboxFilenames[i];
        }
        skyboxTexture = make_shared<Skybox>();
        skyboxTexture->setFilenames(skyboxFilenames);
        skyboxTexture->init();
        skyboxTexture->setUnit(1);
        skyboxTexture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
    }

    void initShadow() {
        switch (SHADOW_QUALITY) {
        case 0:
            SHADOWS = 0;
            SHADOW_AA = 1;
            SHADOW_SIZE = 256;
            break;
        case 1:
            SHADOWS = 1;
            SHADOW_SIZE = 512;
            break;
        case 2:
            SHADOWS = 1;
            SHADOW_SIZE = 1024;
            break;
        case 3:
            SHADOWS = 1;
            SHADOW_SIZE = 2048;
            break;
        case 4:
            SHADOWS = 1;
            SHADOW_SIZE = 4096;
            break;
        default:
            SHADOWS = 1;
            SHADOW_SIZE = 1024;
            break;
        }
        /* set up the FBO for the light's depth */
        // generate the FBO for the shadow depth
        glGenFramebuffers(1, &depthMapFBO);

        // generate the texture
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_SIZE, SHADOW_SIZE, 0,
            GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // bind with framebuffer's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
            depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void setTextureMaterial(int i) {
        // pulled these out of the switch since they were all identical
        //glUniform3f(texProg->getUniform("MatSpec"), 0.2f, 0.2f, 0.2f);
        //glUniform3f(texProg->getUniform("MatAmb"), 0.05f, 0.05f, 0.05f);
        //glUniform1f(texProg->getUniform("Shine"), 32);

        crateAlbedo->bind(texProg->getUniform("albedoMap"));
        crateRoughness->bind(texProg->getUniform("roughnessMap"));
        crateMetallic->bind(texProg->getUniform("metallicMap"));

        switch (i) {
        case 0:
            // marbleTextures[CURRENT_SKIN]->bind(texProg->getUniform("albedoMap"));
            break;
        case 1:
            marbleTextures[CURRENT_SKIN]->bind(texProg->getUniform("albedoMap"));
            break;
        case 2:
            // marbleTextures[CURRENT_SKIN]->bind(texProg->getUniform("albedoMap"));
            break;
        }
    }
    //=================================================
    //=================================================
    // GEOMETRY
    void initGeom(const string &resourceDirectory) {

        loadModels(resourceDirectory);

        loadLevel(resourceDirectory);

        initGameObjects();
    }

    void loadModels(const string &resourceDirectory) {
        initQuad();

        //TODO:: update cube to use texture Coords
        cube = make_shared<Shape>();
        cube->loadMesh(resourceDirectory + "/models/cube.obj");
        cube->resize();
        cube->measure();
        cube->init();

        boxModel = make_shared<Shape>();
        boxModel->loadMesh(resourceDirectory + "/models/box.obj");
        boxModel->measure();
        boxModel->findEdges();
        boxModel->init();

        plane = make_shared<Shape>();
        plane->loadMesh(resourceDirectory + "/models/plane.obj");
        plane->measure();
        plane->init();

        roboHead = make_shared<Shape>();
        roboHead->loadMesh(resourceDirectory + "/models/Robot/RobotHead.obj");
        roboHead->resize();
        roboHead->measure();
        roboHead->init();

        roboLeg = make_shared<Shape>();
        roboLeg->loadMesh(resourceDirectory + "/models/Robot/RobotLeg.obj");
        roboLeg->resize();
        roboLeg->measure();
        roboLeg->init();

        roboFoot = make_shared<Shape>();
        roboFoot->loadMesh(resourceDirectory + "/models/Robot/RobotFoot.obj");
        roboFoot->resize();
        roboFoot->measure();
        roboFoot->init();

        billboard = make_shared<Shape>();
        billboard->loadMesh(resourceDirectory + "/models/billboard.obj");
        billboard->resize();
        billboard->measure();
        billboard->init();

        goalModel = make_shared<Shape>();
        goalModel->loadMesh(resourceDirectory + "/models/goal.obj");
        goalModel->resize();
        goalModel->measure();
        goalModel->init();

        sphere = make_shared<Shape>();
        sphere->loadMesh(resourceDirectory + "/models/quadSphere.obj");
        sphere->resize();
        sphere->measure();
        sphere->init();
    }

    void initGameObjects() {
        ball = make_shared<Ball>(START_POSITION, quat(1, 0, 0, 0), sphere, 1);
        ball->init(windowManager);
        // Control points for enemy's bezier curve path
        std::vector<glm::vec3> enemyPath = {
            vec3{95.0, 2.0, 7.0},
            vec3{100.0, 2.0, 15.0},
            vec3{110.0, 2.0, -1.0},
            vec3{115.0, 2.0, 7.0}
        };
        enemy = make_shared<Enemy>(enemyPath, quat(1, 0, 0, 0), roboHead, roboLeg, roboFoot, 1);
        enemy->init(windowManager);
        enemyPath = {
            vec3{125.0, 8.0, 55.0},
            vec3{115.0, 20.0, 55.0},
            vec3{105.0, 5.0, 55.0},
            vec3{95.0, 8.0, 55.0}
        };
        enemy2 = make_shared<Enemy>(enemyPath, quat(1, 0, 0, 0), roboHead, roboLeg, roboFoot, 1);
        enemy2->init(windowManager);

        goalObject = make_shared<Box>(vec3(0, 11.5, 0), quat(1, 0, 0, 0), goalModel);
        goalObject->scale = vec3(4);

        goal = make_shared<Goal>(goalObject->position + vec3(0, 1, 0), quat(1, 0, 0, 0), nullptr, 1);

        // Need to add each physics object to the octree
        octree = make_shared<Octree>(vec3(-200, -210, -200), vec3(200, 190, 200));
        octree->init(billboard, cube);
        octree->insert(goal);
        octree->insert(goalObject);
        octree->insert(ball);
        octree->insert(boxes);
        octree->insert(enemy);
    }

    void loadLevel(const string &resourceDirectory) {
        ifstream inLevel(resourceDirectory + "/levels/Level1.txt");

        float xval, yval, zval;
        while (inLevel >> xval) {
            inLevel >> yval >> zval;
            auto box = make_shared<Box>(vec3(xval * 8, yval, zval * 6), normalize(quat(0, 0, 0, 0)), boxModel);
            boxes.push_back(box);
        }
    }

    void initQuad() {
        /* set up a quad for rendering a framebuffer */

        // now set up a simple quad for rendering FBO
        glGenVertexArrays(1, &quad_VertexArrayID);
        glBindVertexArray(quad_VertexArrayID);

        static const GLfloat g_quad_vertex_buffer_data[] = {
            -1.0f,
            -1.0f,
            0.0f,
            1.0f,
            -1.0f,
            0.0f,
            -1.0f,
            1.0f,
            0.0f,
            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
        };

        glGenBuffers(1, &quad_vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data),
            g_quad_vertex_buffer_data, GL_STATIC_DRAW);
    }
    //=================================================
    //=================================================
    // RENDERERING

    void render() {
        // Get current frame buffer size.
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);

        mat4 LS;

        if (SHADOW_QUALITY) {
            createShadowMap(&LS);
        }

        glViewport(0, 0, width, height); // frame width and height
        // Clear framebuffer.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (DEBUG_LIGHT) {
            drawDepthMap();
        }
        else {
            renderPlayerView(&LS);
        }
    }

    void drawScene(shared_ptr<Program> shader) {
        // Draw textured models
        if (shader == texProg) {
            glUniform1f(shader->getUniform("shadowSize"), (float)SHADOW_SIZE);
            glUniform1f(shader->getUniform("shadowAA"), (float)SHADOW_AA);
            glUniform1i(shader->getUniform("shadows"), SHADOWS);
        }

        // Create the matrix stacks
        auto M = make_shared<MatrixStack>();

        // Model Identity
        M->pushMatrix();
        M->loadIdentity();

        if (shader == texProg) {
            glUniform3fv(shader->getUniform("viewPos"), 1, value_ptr(camera->eye));
        }
        // =================================================================================================

        // Draw plane
        if (shader == texProg) {
            setTextureMaterial(0);
        }
        M->pushMatrix();
        glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE,
            value_ptr(M->topMatrix()));
        plane->draw(shader);
        M->popMatrix();
        // =================================================================================================

        // Draw ball
        if (shader == texProg) {
            setTextureMaterial(1);
        }
        ball->draw(shader, M);
        goalObject->draw(shader, M);
        enemy->draw(shader, M);
        enemy2->draw(shader, M);
        // =================================================================================================

        // Draw Boxes
        if (shader == texProg) {
            setTextureMaterial(2);
        }
        for (auto box : boxes) {
            box->draw(shader, M);
        }
        // =================================================================================================

        // cleanup
        M->popMatrix();
        // =================================================================================================
    }

    void createShadowMap(mat4 *LS) {
        // set up light's depth map
        glViewport(0, 0, SHADOW_SIZE, SHADOW_SIZE); // shadow map width and height
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        // set up shadow shader
        // render scene
        DepthProg->bind();
        // TODO you will need to fix these
        mat4 LP = SetOrthoMatrix(DepthProg);
        mat4 LV = SetLightView(DepthProg, gameLight, vec3(60, 0, 0), vec3(0, 1, 0));
        *LS = LP * LV;
        // SetLightView(DepthProg, gameLight, g_lookAt, vec3(0, 1, 0));
        drawScene(DepthProg);
        DepthProg->unbind();
        glCullFace(GL_BACK);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void drawDepthMap() {
        /* code to draw the light depth buffer */
        // geometry style debug on light - test transforms, draw geometry from
        // light perspective
        if (GEOM_DEBUG) {
            DepthProgDebug->bind();
            // render scene from light's point of view
            SetOrthoMatrix(DepthProgDebug);
            SetLightView(DepthProgDebug, gameLight, vec3(60, 0, 0), vec3(0, 1, 0));
            drawScene(DepthProgDebug);
            DepthProgDebug->unbind();
        }
        else {
            // actually draw the light depth map
            DebugProg->bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            glUniform1i(DebugProg->getUniform("texBuf"), 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableVertexAttribArray(0);
            DebugProg->unbind();
        }
    }

    void drawSkyBox() {
        // Draw skybox
        skyProg->bind();
        setProjectionMatrix(skyProg);
        setView(skyProg);

        skyboxTexture->bind(skyProg->getUniform("Texture0"));
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        cube->draw(skyProg);
        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);

        skyProg->unbind();
    }

    void sendShadowMap() {
        /* also set up light depth map */
        glActiveTexture(GL_TEXTURE30);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glUniform1i(texProg->getUniform("shadowDepth"), 0);
    }

    void renderPlayerView(mat4 *LS) {
        drawSkyBox();

        texProg->bind();

        setLight(texProg);
        setProjectionMatrix(texProg);
        setView(texProg);

        sendShadowMap();

        glUniformMatrix4fv(texProg->getUniform("LS"), 1, GL_FALSE, value_ptr(*LS));
        drawScene(texProg);

        texProg->unbind();

        if (octree->debug) {
            drawOctree();
        }
    }

    void drawOctree() {
        circleProg->bind();
        setProjectionMatrix(circleProg);
        setView(circleProg);
        octree->drawDebugBoundingSpheres(circleProg);
        circleProg->unbind();

        cubeOutlineProg->bind();
        setProjectionMatrix(cubeOutlineProg);
        setView(cubeOutlineProg);
        octree->drawDebugOctants(cubeOutlineProg);
        cubeOutlineProg->unbind();
    }
    //=================================================
    //=================================================
    // GENERAL HELPERS

    void update(float dt)
    {
        octree->update();

        if (ballInGoal && !DID_WIN) {
            DID_WIN = true;
            cout << "✼　 ҉ 　✼　 ҉ 　✼" << endl;
            cout << "You win!" << endl;
            cout << "Time: " << glfwGetTime() - START_TIME << endl;
            cout << "✼　 ҉ 　✼　 ҉ 　✼" << endl;
        }
        auto boxesToCheck = octree->query(ball);
        for (auto box : boxesToCheck) {
            box->checkCollision(ball.get());
        }

        for (auto box : boxes) {
            box->update(dt);
        }
        //TODO:: Do Collision checks between ball and Enemy
        /*
        auto enemiesToCheck = octree->query(ball);
        for (auto enemies : enemiesToCheck) {
            
        }*/
        
        goalObject->update(dt);
        ball->update(dt, camera->getDolly(), camera->getStrafe());
        camera->update(dt, ball);
        goal->update(dt);
        enemy->update(dt);
        enemy2->update(dt);

        viewFrustum.extractPlanes(setProjectionMatrix(nullptr), setView(nullptr));
        octree->markInView(viewFrustum);
    }

    void setLight(shared_ptr<Program> prog) {
        glUniform3f(prog->getUniform("lightPosition"), gameLight.x, gameLight.y, gameLight.z);
        glUniform3f(prog->getUniform("lightColor"), gameLightColor.x, gameLightColor.y, gameLightColor.z);
    }

    mat4 SetOrthoMatrix(shared_ptr<Program> curShade) {
        // shadow mapping helper

        mat4 ortho = glm::ortho(-96.0f, 96.0f, -96.0f, 96.0f, 0.1f, 500.0f);
        // fill in the glUniform call to send to the right shader!
        glUniformMatrix4fv(curShade->getUniform("LP"), 1, GL_FALSE,
            value_ptr(ortho));
        return ortho;
    }

    mat4 SetLightView(shared_ptr<Program> curShade, vec3 pos, vec3 LA, vec3 up) {
        // shadow mapping helper

        mat4 Cam = lookAt(pos, LA, up);
        // fill in the glUniform call to send to the right shader!
        glUniformMatrix4fv(curShade->getUniform("LV"), 1, GL_FALSE, value_ptr(Cam));
        return Cam;
    }

    mat4 setProjectionMatrix(shared_ptr<Program> curShade) {
        int width, height;
        glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
        float aspect = width / (float)height;
        mat4 Projection = perspective(radians(50.0f), aspect, 0.1f, 200.0f);
        if (curShade != nullptr)
        {
            glUniformMatrix4fv(curShade->getUniform("P"), 1, GL_FALSE,
                value_ptr(Projection));
        }
        return Projection;
    }

    mat4 setView(shared_ptr<Program> curShade) {
        mat4 Cam = lookAt(camera->eye, camera->lookAtPoint, camera->upVec);
        if (curShade != nullptr)
        {
            glUniformMatrix4fv(curShade->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
        }
        return Cam;
    }

    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        //skin switching key call back
        if (key == GLFW_KEY_O && action == GLFW_PRESS) {
            CURRENT_SKIN = (CURRENT_SKIN + 1) % NUMBER_OF_MARBLE_SKINS;

            
        }
        else if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
            SHADOW_AA = (SHADOW_AA + 1) % 9;
            if (SHADOW_AA == 0) {
                SHADOW_AA++;
            }
        }
        else if (key == GLFW_KEY_T && action == GLFW_PRESS) {
            SHADOWS = !SHADOWS;
        }
        // other call backs
        else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR,
                GLFW_CURSOR_NORMAL);
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        else if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        else if (key == GLFW_KEY_V && action == GLFW_PRESS) {
            camera->flying = !camera->flying;
        }
        else if (key == GLFW_KEY_U && action == GLFW_PRESS) {
            DEBUG_LIGHT = !DEBUG_LIGHT;
        }
        else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            SHOW_CURSOR = !SHOW_CURSOR;
            if (SHOW_CURSOR) {
                glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else {
                glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
        }
        else if (key == GLFW_KEY_H && action == GLFW_PRESS) {
            octree->debug = !octree->debug;
        }
        else if (key == GLFW_KEY_R && action == GLFW_PRESS) {
            ball->position = START_POSITION;
            ball->velocity = vec3(0);
            DID_WIN = false;
            ballInGoal = false;
            START_TIME = glfwGetTime();
        }
        else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
            camera->previewLvl = !camera->previewLvl;
            if (camera->previewLvl){
                camera->startLvlPreview(CENTER_LVL_POSITION);
            }
        }
    }

    void scrollCallback(GLFWwindow *window, double deltaX, double deltaY)
    {
        float newDistance = deltaY + camera->distToBall;
        if (newDistance < 15 && newDistance > 2){
            camera->distToBall += deltaY;
        }
        //cout<< "DistToBall: " << camera->distToBall << endl;
    }

    void mouseCallback(GLFWwindow *window, int button, int action, int mods) {
        double posX, posY;

        if (action == GLFW_PRESS) {
            MOUSE_DOWN = true;
            glfwGetCursorPos(window, &posX, &posY);
            cout << "Pos X " << posX << " Pos Y " << posY << endl;
            cout << "" << ball->position.x << ", " << ball->position.y << ", " << ball->position.z << endl;
            MOVING = true;
        }

        if (action == GLFW_RELEASE) {
            MOVING = false;
            MOUSE_DOWN = false;
        }
    }

    void resizeCallback(GLFWwindow *window, int width, int height) {
        glViewport(0, 0, width, height);
    }
    //=================================================
};

int main(int argc, char **argv) {
    // Where the resources are loaded from
    std::string resourceDir = "../Resources";

    if (argc >= 2) {
        resourceDir = argv[1];
    }

    Application *application = new Application();

    // Your main will always include a similar set up to establish your window
    // and GL context, etc.

    WindowManager *windowManager = new WindowManager();
    windowManager->init(1280, 720);
    // windowManager->init(1920, 1080);
    // windowManager->init(2560, 1440);
    windowManager->setEventCallbacks(application);
    application->windowManager = windowManager;

    // This is the code that will likely change program to program as you
    // may need to initialize or set up different data and state

    application->init(resourceDir);
    application->initShaders(resourceDir);
    application->initTextures(resourceDir);
    application->initGeom(resourceDir);

    application->START_TIME = glfwGetTime();

    double t = 0;
    const double dt = 0.02;
    double currentTime = application->START_TIME;
    double accumulator = 0;

    // Loop until the user closes the window.
    while (!glfwWindowShouldClose(windowManager->getHandle())) {
        // Render scene.
        double newTime = glfwGetTime();
        double frameTime = newTime - currentTime;
        currentTime = newTime;

        accumulator += frameTime;

        while (accumulator >= dt)
        {
            application->update(dt);
            accumulator -= dt;
            t += dt;
        }

        application->render();

        // Swap front and back buffers.
        glfwSwapBuffers(windowManager->getHandle());
        // Poll for and process events.
        glfwPollEvents();
    }

    // Quit program.
    windowManager->shutdown();
    return 0;
}
