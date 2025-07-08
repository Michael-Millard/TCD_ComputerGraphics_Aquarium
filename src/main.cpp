#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <my_shader.h>
#include <my_camera.h>
#include <my_model.h>

#include <iostream>
#include <random>
#define _USE_MATH_DEFINES
#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Callback function declarations
void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xIn, double yIn);
void scrollCallback(GLFWwindow* window, double xOff, double yOff);
void processUserInput(GLFWwindow* window);

// Camera specs (set later, can't call functions here)
const float cameraSpeed = 2.0f;
const float mouseSensitivity = 0.1f;
const float cameraZoom = 50.0f;
const float xPosInit = 0.0f;
const float yPos = 1.8f;
const float zPosInit = 9.0f;
Camera camera(glm::vec3(xPosInit, yPos, zPosInit));

// Global params
unsigned int SCREEN_WIDTH = 1920;
unsigned int SCREEN_HEIGHT = 1080;
bool firstMouse = true;

float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate to the left
float pitch = 0.0f;
float xPrev = static_cast<float>(SCREEN_WIDTH) / 2.0f;
float yPrev = static_cast<float>(SCREEN_HEIGHT) / 2.0f;

// Timing params
float deltaTime = 0.0f;	// time between current frame and previous frame
float prevFrame = 0.0f;

// For random placement of models
float generateRandomNumInRange(float low, float high)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(low, high);

    return dis(gen);
}

// 3D model names
#define MODEL_FLOOR "models/floor.obj"
#define MODEL_WALLS "models/walls.obj"
#define MODEL_ROOF "models/roof.obj"
#define MODEL_FISH_TANK "models/fish_tank.obj"
#define MODEL_ROOF_LAMP "models/roof_lamp.obj"
#define MODEL_KELP "models/kelp.obj"
#define MODEL_JELLYFISH "models/jellyfish.obj"
#define MODEL_JELLYFISH2 "models/jellyfish2.obj"
#define MODEL_DIRT_FLOOR "models/dirt_floor.obj"
#define MODEL_ROCK "models/rock.obj"
#define MODEL_FISH1 "models/fish1.obj"
#define MODEL_FISH2 "models/fish2.obj"
#define MODEL_VOLCANO "models/volcano.obj"
#define MODEL_FISH_FOOD "models/fish_food.obj"
#define MODEL_SHARK "models/shark.obj"
#define MODEL_PAINTING "models/painting1.obj"
#define MODEL_TABLES "models/tables.obj"

// Function to init models
Model initModel(Model _model, const float _tX, const float _tY, 
    const float _tZ, const float _rX, const float _rY, const float _rZ)
{
    Model model = _model;
    for (unsigned int i = 0; i < static_cast<unsigned int>(model.meshes.size()); i++)
    {
        // Base mesh, rotated and translated
        if (i == 0)
        {
            // Set 6 DoF pose params
            model.meshes[i].mesh6DoF[tX] = _tX; model.meshes[i].mesh6DoF[tY] = _tY; model.meshes[i].mesh6DoF[tZ] = _tZ;
            model.meshes[i].mesh6DoF[rX] = _rX; model.meshes[i].mesh6DoF[rY] = _rY; model.meshes[i].mesh6DoF[rZ] = _rZ;

            // Rotations in radians, X, Y, then Z
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), _rX, glm::vec3(1.0f, 0.0f, 0.0f));
            rotation = glm::rotate(rotation, _rY, glm::vec3(0.0f, 1.0f, 0.0f));
            rotation = glm::rotate(rotation, _rZ, glm::vec3(0.0f, 0.0f, 1.0f));

            // Translation
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(tX, tY, tZ));
            model.meshes[i].meshMatrix = rotation * translation;
        }
        // Rest of meshes lower in hierarchy, identity for now
        else
            model.meshes[i].meshMatrix = glm::mat4(1);

        model.meshes[i].updateModelMatrix();
    }
    return model;
}

// Fish food animation
bool fishFoodInit = false;
bool fishFoodAnimStarted = false;

// Wall constrains function
glm::vec4 getWallConstraints(std::vector<glm::vec3> modelVertices)
{
    float xMin{}, xMax{}, zMin{}, zMax{};
    for (int i = 0; i < (int)modelVertices.size(); i++)
    {
        // If first loop, just set
        if (i == 0)
        {
            xMin = xMax = modelVertices[i].x;
            zMin = zMax = modelVertices[i].z;
            continue;
        }

        // Else compare to find constraints
        if (modelVertices[i].x < xMin)
            xMin = modelVertices[i].x;
        if (modelVertices[i].x > xMax)
            xMax = modelVertices[i].x;
        if (modelVertices[i].z < zMin)
            zMin = modelVertices[i].z;
        if (modelVertices[i].z > zMax)
            zMax = modelVertices[i].z;
    }

    // Adjust slightly so not exactly "in wall"
    xMin += 0.25f; xMax -= 0.25f;
    zMin += 0.25f; zMax -= 0.25f;

    return glm::vec4(xMin, xMax, zMin, zMax);
}

// Main function
int main()
{
    // glfw init and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, NULL); // Remove title bar

    // Screen params
    GLFWmonitor* MyMonitor = glfwGetPrimaryMonitor(); 
    const GLFWvidmode* mode = glfwGetVideoMode(MyMonitor);
    SCREEN_WIDTH = mode->width; SCREEN_HEIGHT = mode->height;

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Acquarium Scene", glfwGetPrimaryMonitor(), nullptr);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Callback functions
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Mouse capture
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Load all OpenGL function pointers with GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);    // Depth-testing
    glDepthFunc(GL_LESS);       // Smaller value as "closer" for depth-testing

    // Build and compile shaders
    Shader shader("shaders/projectVertexShader.vs", "shaders/projectFragmentShader.fs");

    // Load models
    Model floorModel(MODEL_FLOOR);
    Model wallModel(MODEL_WALLS);
    Model roofModel(MODEL_ROOF);
    Model fishTankModel(MODEL_FISH_TANK);
    Model roofLampModel(MODEL_ROOF_LAMP);
    Model kelpModel(MODEL_KELP);
    Model jellyfishModel(MODEL_JELLYFISH);
    Model jellyfish2Model(MODEL_JELLYFISH2);
    Model dirtFloorModel(MODEL_DIRT_FLOOR);
    Model rockModel(MODEL_ROCK);
    Model fish1Model(MODEL_FISH1);
    Model fish2Model(MODEL_FISH2);
    Model volcanoModel(MODEL_VOLCANO);
    Model fishFoodModel(MODEL_FISH_FOOD);
    Model sharkModel(MODEL_SHARK);
    Model paintingModel(MODEL_PAINTING);
    Model tablesModel(MODEL_TABLES);

    // Create 150 kelp models, each with 8 segments
    std::vector<Model> kelpModels;
    for (int i = 0; i < 150; i++)
        kelpModels.push_back(initModel(kelpModel, generateRandomNumInRange(-5.25f, 5.25f), 0.0f, generateRandomNumInRange(-5.25f, 5.25f),
            0.0f, glm::radians(generateRandomNumInRange(0.0f, 180.0f)), 0.0f));

    // Create 20 jellyfish 1s
    std::vector<Model> jellyfish1Models;
    for (int i = 0; i < 20; i++)
        jellyfish1Models.push_back(initModel(jellyfishModel, generateRandomNumInRange(-5.25f, 5.25f), generateRandomNumInRange(0.5f, 2.5f), generateRandomNumInRange(-5.25f, 5.25f),
            0.0f, glm::radians(generateRandomNumInRange(0.0f, 180.0f)), 0.0f));

    // Create 20 jellyfish 2s
    std::vector<Model> jellyfish2Models;
    for (int i = 0; i < 20; i++)
        jellyfish2Models.push_back(initModel(jellyfish2Model, generateRandomNumInRange(-5.25f, 5.25f), generateRandomNumInRange(0.5f, 2.5f), generateRandomNumInRange(-5.25f, 5.25f),
            0.0f, glm::radians(generateRandomNumInRange(0.0f, 180.0f)), 0.0f));

    // Shark model
    sharkModel = initModel(sharkModel, 4.5f, generateRandomNumInRange(1.0f, 2.0f), 4.5f,
        0.0f, glm::radians(generateRandomNumInRange(175.0f, 185.0f)), 0.0f);
    sharkModel.meshes[0].initRad = std::sqrtf(std::powf(sharkModel.meshes[0].mesh6DoF[tX], 2.0f) + std::powf(sharkModel.meshes[0].mesh6DoF[tZ], 2.0f));
    sharkModel.meshes[0].initRot = sharkModel.meshes[0].mesh6DoF[rY];

    // 75 fish 1s
    std::vector<Model> fish1Models;
    for (int i = 0; i < 75; i++)
    {
        fish1Models.push_back(initModel(fish1Model, generateRandomNumInRange(-5.25f, 5.25f), generateRandomNumInRange(0.5f, 2.8f), generateRandomNumInRange(-5.25f, 5.25f),
            0.0f, glm::radians(generateRandomNumInRange(175.0f, 185.0f)), 0.0f));
        fish1Models[i].meshes[0].initRad = std::sqrtf(std::powf(fish1Models[i].meshes[0].mesh6DoF[tX], 2.0f) + std::powf(fish1Models[i].meshes[0].mesh6DoF[tZ], 2.0f));
        fish1Models[i].meshes[0].initRot = fish1Models[i].meshes[0].mesh6DoF[rY];
    }

    // 75 fish 2s
    std::vector<Model> fish2Models;
    for (int i = 0; i < 75; i++)
    {
        fish2Models.push_back(initModel(fish2Model, generateRandomNumInRange(-5.25f, 5.25f), generateRandomNumInRange(0.5f, 2.8f), generateRandomNumInRange(-5.25f, 5.25f),
            0.0f, glm::radians(generateRandomNumInRange(175.0f, 185.0f)), 0.0f));
        fish2Models[i].meshes[0].initRad = std::sqrtf(std::powf(fish2Models[i].meshes[0].mesh6DoF[tX], 2.0f) + std::powf(fish2Models[i].meshes[0].mesh6DoF[tZ], 2.0f));
        fish2Models[i].meshes[0].initRot = fish2Models[i].meshes[0].mesh6DoF[rY];
    }

    // 15 rocks
    std::vector<Model> rockModels;
    for (int i = 0; i < 15; i++)
        rockModels.push_back(initModel(rockModel, generateRandomNumInRange(-5.0f, 5.0f), 0.0f, generateRandomNumInRange(-5.0f, 5.0f),
            0.0f, glm::radians(generateRandomNumInRange(0.0f, 180.0f)), 0.0f));

    // Set wall constrains
    std::vector<glm::vec3> wallVertices = {};
    for (const Mesh& mesh: wallModel.meshes)
    {
        for (const Vertex& vertex: mesh.vertices)
            wallVertices.push_back(vertex.Position);
    }
    camera.setWallConstrains(getWallConstraints(wallVertices));

    // Fine tune camera params
    camera.setMouseSensitivity(mouseSensitivity);
    camera.setCameraMovementSpeed(cameraSpeed);
    camera.setZoom(cameraZoom);
    camera.setFPSCamera(true, yPos);
    camera.setZoomEnabled(false);

    // Point light locations
    glm::vec3 lightPositions[5] =
    {
        glm::vec3(0.0f, 3.0f, 0.0f),
        glm::vec3(-3.0f, 3.0f, -3.0f),
        glm::vec3(3.0f, 3.0f, -3.0f),
        glm::vec3(-3.0f, 3.0f, 3.0f),
        glm::vec3(3.0f, 3.0f, 3.0f),
    };

    shader.use();

    // Lights in tank
    for (int i = 0; i < 5; i++)
    {
        shader.setVec3("pointLights[" + std::to_string(i) + "].position", lightPositions[i]);
        shader.setVec3("pointLights[" + std::to_string(i) + "].ambient", glm::vec3(0.1f, 0.2f, 0.4f)); 
        shader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", glm::vec3(0.8f, 0.8f, 0.8f)); 
        shader.setVec3("pointLights[" + std::to_string(i) + "].specular", glm::vec3(0.5f, 0.5f, 0.5f));
        shader.setFloat("pointLights[" + std::to_string(i) + "].constant", 0.9f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.04f);
        shader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.01f);
    }

    // Set specular exponent
    shader.setFloat("specularExponent", 32.0f);

    // Render loop
    float elapsedTime = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        // Per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - prevFrame;
        elapsedTime += deltaTime;
        prevFrame = currentFrame;

        // User input handling
        processUserInput(window);

        // Clear screen colour and buffers
        glClearColor(0.2f, 0.5f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Enable shader before setting uniforms
        shader.use();
        shader.setBool("useTexture", GL_TRUE);

        // Camera position
        shader.setVec3("viewPositon", camera.position);

        // Model, View & Projection transformations, set uniforms in shader
        glm::mat4 model = glm::identity<glm::mat4>();
        shader.setMat4("model", model);
        glm::mat4 view = camera.getViewMatrix();
        shader.setMat4("view", view);
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT), 0.1f, 100.0f);
        shader.setMat4("projection", projection);

        // Fish food animation (if clicked)
        if (fishFoodAnimStarted)
        {
            if (!fishFoodInit)
            {
                fishFoodModel = initModel(fishFoodModel, 4.0f, 2.5f, 4.0f, 0.0f, 0.0f, 0.0f);
                for (unsigned int i = 1; i < static_cast<unsigned int>(fishFoodModel.meshes.size()); i++)
                {
                    fishFoodModel.meshes[i].mesh6DoF[tX] = fishFoodModel.meshes[0].mesh6DoF[tX];
                    fishFoodModel.meshes[i].mesh6DoF[tY] = fishFoodModel.meshes[0].mesh6DoF[tY];
                    fishFoodModel.meshes[i].mesh6DoF[tZ] = fishFoodModel.meshes[0].mesh6DoF[tZ];
                    fishFoodModel.meshes[i].mesh6DoF[rX] = fishFoodModel.meshes[0].mesh6DoF[rX];
                    fishFoodModel.meshes[i].mesh6DoF[rY] = fishFoodModel.meshes[0].mesh6DoF[rY];
                    fishFoodModel.meshes[i].mesh6DoF[rZ] = fishFoodModel.meshes[0].mesh6DoF[rZ];
                    fishFoodModel.meshes[i].updateModelMatrix();
                }
                fishFoodInit = true;

                model = fishFoodModel.meshes[0].meshMatrix;
                shader.setMat4("model", model);
                fishFoodModel.draw(shader);
            }
            else
            {
                for (unsigned int i = 0; i < static_cast<unsigned int>(fishFoodModel.meshes.size()); i++)
                {
                    // Check when to end animation
                    if ((fishFoodModel.meshes[i].mesh6DoF[tY] - elapsedTime * 0.0001f) < 0.0f)
                    {
                        fishFoodAnimStarted = false;
                        fishFoodInit = false;
                    }

                    fishFoodModel.meshes[i].mesh6DoF[tY] -= elapsedTime * 0.00002f;
                    fishFoodModel.meshes[i].updateModelMatrix();

                    model = fishFoodModel.meshes[i].meshMatrix;
                    shader.setMat4("model", model);
                    fishFoodModel.meshes[i].draw(shader);
                }

                // Draw shark
                for (unsigned int j = 0; j < static_cast<unsigned int>(sharkModel.meshes.size()); j++)
                {
                    // Update shark pose params
                    glm::vec3 directionVec(fishFoodModel.meshes[0].mesh6DoF[tX] - sharkModel.meshes[0].mesh6DoF[tX],
                        fishFoodModel.meshes[0].mesh6DoF[tY] - sharkModel.meshes[0].mesh6DoF[tY],
                        fishFoodModel.meshes[0].mesh6DoF[tZ] - sharkModel.meshes[0].mesh6DoF[tZ]);

                    float magnitude = std::sqrtf(std::powf(directionVec[0], 2.0f) + std::powf(directionVec[1], 2.0f) + std::powf(directionVec[2], 2.0f));

                    // Check if shark has got to food to end animation
                    if (magnitude < 0.1f)
                    {
                        fishFoodAnimStarted = false;
                        fishFoodInit = false;
                        sharkModel.meshes[0].initRad = std::sqrtf(std::powf(sharkModel.meshes[0].mesh6DoF[tX], 2.0f) + std::powf(sharkModel.meshes[0].mesh6DoF[tZ], 2.0f));
                        break;
                    }

                    // Normalize to unit vec
                    directionVec /= magnitude;

                    sharkModel.meshes[0].mesh6DoF[tX] += directionVec[0] * 0.005f;
                    sharkModel.meshes[0].mesh6DoF[tY] += directionVec[1] * 0.005f;
                    sharkModel.meshes[0].mesh6DoF[tZ] += directionVec[2] * 0.005f;

                    // Slowly rotate towards target
                    float target = std::atan2f(directionVec[2], directionVec[0]) + float(M_PI);
                    if ((target - sharkModel.meshes[0].mesh6DoF[rY]) < 0.0f)
                        sharkModel.meshes[0].mesh6DoF[rY] -= 0.001f;
                    else
                        sharkModel.meshes[0].mesh6DoF[rY] += 0.001f;


                    for (unsigned int j = 0; j < static_cast<unsigned int>(sharkModel.meshes.size()); j++)
                    {
                        sharkModel.meshes[j].mesh6DoF[rY] = sharkModel.meshes[0].mesh6DoF[rY] + 0.1f * sin(elapsedTime * 5.0f + j * 5.0f);
                        sharkModel.meshes[j].updateModelMatrix();

                        // Set model matrix (multiply with previous model matrix for hierarchy animation)
                        model = sharkModel.meshes[j].meshMatrix;
                        shader.setMat4("model", model);
                        sharkModel.meshes[j].draw(shader);

                        // Remove "wagging" for next loop
                        sharkModel.meshes[j].mesh6DoF[rY] -= 0.1f * sin(elapsedTime * 5.0f + j * 5.0f);
                    }
                }
            }
        }
        else
        {
            // Draw shark
            for (unsigned int j = 0; j < static_cast<unsigned int>(sharkModel.meshes.size()); j++)
            {
                // Update shark pose params
                float sharkRad = sharkModel.meshes[0].initRad;
                float theta = elapsedTime * 0.1f;
                sharkModel.meshes[0].mesh6DoF[tX] = sharkRad * cos(theta);
                sharkModel.meshes[0].mesh6DoF[tZ] = sharkRad * sin(theta);
                sharkModel.meshes[0].mesh6DoF[rY] = (float(M_PI) / 2.0f) - theta + float(M_PI);

                for (unsigned int j = 0; j < static_cast<unsigned int>(sharkModel.meshes.size()); j++)
                {
                    sharkModel.meshes[j].mesh6DoF[rY] = sharkModel.meshes[0].mesh6DoF[rY] + 0.1f * sin(elapsedTime * 5.0f + j * 5.0f);
                    sharkModel.meshes[j].updateModelMatrix();

                    // Set model matrix (multiply with previous model matrix for hierarchy animation)
                    model = sharkModel.meshes[j].meshMatrix;
                    shader.setMat4("model", model);
                    sharkModel.meshes[j].draw(shader);
                }
            }
        }

        // Draw fish 1s
        model = glm::mat4(1);
        for (unsigned int i = 0; i < static_cast<unsigned int>(fish1Models.size()); i++)
        {
            // Update fish pose params
            float fishRad = fish1Models[i].meshes[0].initRad;
            float theta = elapsedTime * 0.1f + 1.0 * i;
            fish1Models[i].meshes[0].mesh6DoF[tX] = fishRad * cos(theta);
            fish1Models[i].meshes[0].mesh6DoF[tZ] = fishRad * sin(theta);
            fish1Models[i].meshes[0].mesh6DoF[rY] = (float(M_PI) / 2.0f) - theta + float(M_PI);

            for (unsigned int j = 0; j < static_cast<unsigned int>(fish1Models[i].meshes.size()); j++)
            {
                fish1Models[i].meshes[j].mesh6DoF[rY] = fish1Models[i].meshes[0].mesh6DoF[rY] + 0.1f * sin(elapsedTime * 5.0f + j * 5.0f);
                fish1Models[i].meshes[j].updateModelMatrix();

                // Set model matrix (multiply with previous model matrix for hierarchy animation)
                model = fish1Models[i].meshes[j].meshMatrix;
                shader.setMat4("model", model);
                fish1Models[i].meshes[j].draw(shader);
            }

            // Reset kelp hierarchy matrix for next kelp model
            model = glm::mat4(1);
        }

        // Draw fish 2s
        model = glm::mat4(1);
        for (unsigned int i = 0; i < static_cast<unsigned int>(fish2Models.size()); i++)
        {
            // Update fish pose params
            float fishRad = fish2Models[i].meshes[0].initRad;
            float theta = elapsedTime * 0.1f + 1.0 * i;
            fish2Models[i].meshes[0].mesh6DoF[tX] = fishRad * cos(theta);
            fish2Models[i].meshes[0].mesh6DoF[tZ] = fishRad * sin(theta);
            fish2Models[i].meshes[0].mesh6DoF[rY] = (float(M_PI) / 2.0f) - theta + float(M_PI);

            for (unsigned int j = 0; j < static_cast<unsigned int>(fish2Models[i].meshes.size()); j++)
            {
                fish2Models[i].meshes[j].mesh6DoF[rY] = fish2Models[i].meshes[0].mesh6DoF[rY] + 0.1f * sin(elapsedTime * 5.0f + j * 5.0f);
                fish2Models[i].meshes[j].updateModelMatrix();

                // Set model matrix (multiply with previous model matrix for hierarchy animation)
                model = fish2Models[i].meshes[j].meshMatrix;
                shader.setMat4("model", model);
                fish2Models[i].meshes[j].draw(shader);
            }

            // Reset kelp hierarchy matrix for next kelp model
            model = glm::mat4(1);
        }

        // Draw jellyfish 1s
        for (unsigned int i = 0; i < static_cast<unsigned int>(jellyfish1Models.size()); i++)
        {
            // Update jellyfish pose params
            jellyfish1Models[i].meshes[0].mesh6DoF[tY] = 0.5f * sin(elapsedTime * 0.5f - i * 0.5f) + 1.5f;
            jellyfish1Models[i].meshes[0].mesh6DoF[rY] += glm::radians(0.3f);
            jellyfish1Models[i].meshes[0].updateModelMatrix();

            // Set model matrix
            model = jellyfish1Models[i].meshes[0].meshMatrix;
            shader.setMat4("model", model);
            jellyfish1Models[i].draw(shader);
        }

        // Draw jellyfish 2s
        for (unsigned int i = 0; i < static_cast<unsigned int>(jellyfish2Models.size()); i++)
        {
            // Update jellyfish pose params
            jellyfish2Models[i].meshes[0].mesh6DoF[tY] = 0.5f * sin(elapsedTime * 0.5f - i * 0.5f) + 1.5f;
            jellyfish2Models[i].meshes[0].mesh6DoF[rY] += glm::radians(0.3f);
            jellyfish2Models[i].meshes[0].updateModelMatrix();

            // Set model matrix
            model = jellyfish2Models[i].meshes[0].meshMatrix;
            shader.setMat4("model", model);
            jellyfish2Models[i].draw(shader);
        }

        // Draw kelp
        model = glm::mat4(1);
        for (unsigned int i = 0; i < static_cast<unsigned int>(kelpModels.size()); i++)
        {
            for (unsigned int j = 0; j < static_cast<unsigned int>(kelpModels[i].meshes.size()); j++)
            {
                // Update jellyfish pose params
                kelpModels[i].meshes[j].mesh6DoF[rZ] = 0.05f * sin(elapsedTime * 0.75f + j * 0.5f);
                kelpModels[i].meshes[j].updateModelMatrix();

                // Set model matrix (multiply with previous model matrix for hierarchy animation)
                model *= kelpModels[i].meshes[j].meshMatrix;
                shader.setMat4("model", model);
                kelpModels[i].meshes[j].draw(shader);
            }

            // Reset kelp hierarchy matrix for next kelp model
            model = glm::mat4(1);
        }

        // Draw rocks
        for (unsigned int i = 0; i < static_cast<unsigned int>(rockModels.size()); i++)
        {
            model = rockModels[i].meshes[0].meshMatrix;
            shader.setMat4("model", model);
            rockModels[i].draw(shader);
        }

        // Reset model matrix to identity
        model = glm::mat4(1);
        shader.setMat4("model", model);

        // Draw
        floorModel.draw(shader);
        wallModel.draw(shader);
        tablesModel.draw(shader);
        roofLampModel.draw(shader);
        roofModel.draw(shader);
        dirtFloorModel.draw(shader);
        volcanoModel.draw(shader);
        paintingModel.draw(shader);

        glDepthMask(GL_FALSE);  // Disable depth writes for glass
        shader.setBool("useTexture", GL_FALSE);
        shader.setVec4("glassColor", glm::vec4(0.8f, 0.8f, 0.9f, 0.2f)); // Glass, 20% transparent, light blue
        fishTankModel.draw(shader);
        glDepthMask(GL_TRUE);   // Enable depth writes after glass

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate and return success
    glfwTerminate();
    return 0;
}

// Process keyboard inputs
void processUserInput(GLFWwindow* window)
{
    // Escape to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // WASD to move, parse to camera processing commands
    // Positional constraints implemented in camera class
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboardInput(GLFW_KEY_D, deltaTime);

    // Fish food (F)
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        // Can't keep spamming it
        if (!fishFoodAnimStarted)
            fishFoodAnimStarted = true;
    }
}

// Window size change callback
void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Ensure viewport matches new window dimensions
    glViewport(0, 0, width, height);

    // Adjust screen width and height params that set the aspect ratio in the projection matrix
    SCREEN_WIDTH = width;
    SCREEN_HEIGHT = height;
}

// Mouse input callback
void mouseCallback(GLFWwindow* window, double xIn, double yIn)
{
    // Cast doubles to floats
    float x = static_cast<float>(xIn);
    float y = static_cast<float>(yIn);

    // Check if first time this callback function is being used, set last variables if so
    if (firstMouse)
    {
        xPrev = x;
        yPrev = y;
        firstMouse = false;
    }

    // Compute offsets relative to last positions
    float xOff = x - xPrev;
    // Reverse since y-coordinates are inverted (bottom to top)
    float yOff = yPrev - y; 
    xPrev = x; yPrev = y;

    // Tell camera to process new mouse offsets
    camera.processMouseMovement(xOff, yOff);
}

// Mouse scroll wheel input callback - camera zoom must be enabled for this to work
void scrollCallback(GLFWwindow* window, double xOff, double yOff)
{
    // Tell camera to process new y-offset from mouse scroll whell
    camera.processMouseScroll(static_cast<float>(yOff));
}