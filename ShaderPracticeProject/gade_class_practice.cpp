#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <boost/algorithm/string.hpp>


#include "shader.h"
#include "camera.h"
#include <iostream>
#include "model.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <list>
#include <time.h>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void loadRoomData();
void loadSettings();
void loadCubeVerticies();
unsigned int loadTexture(const char* path);
void character_callback(GLFWwindow* window, unsigned int codepoint);
void excecuteCommand(std::vector<std::string>commands);
void countTrangles(int addedNum);
void loadSkyboxVertices();
void loadSkyboxFaces(string folder);
unsigned int loadCubemap(vector<std::string> faces);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
std::vector<std::string> parse(std::string s);

/*
    E = East Pointing wall
    N = North Point wall
    L = Point Light
    ^ = Easting Point Door
    < = North Point Door
    * = Floor without roof
    & = Floor with roof
    */
//room data lists
std::vector<glm::vec3> floorNoRoof;
std::vector<glm::vec3> floorWithRoof;
std::vector<glm::vec3> eastWalls;
std::vector<glm::vec3> eastSmallWalls;
std::vector<glm::vec3> northWalls;
std::vector<glm::vec3> northSmallWalls;
std::vector<glm::vec3> pointLights;
std::vector<glm::vec3> eastDoors;
std::vector<glm::vec3> northDoors;
std::vector<glm::vec3> cornerWalls;
std::vector<glm::vec3> cornerSmallWalls;
std::vector<glm::vec3> impSpawns;
std::vector<glm::vec3> backpackSpawns;
std::vector<glm::vec3> preloadedScaleValues;
vector<std::string> skyboxFaces;

std::vector<Model> spawnedModels;
std::vector<glm::vec3> spawnLocations;
int spawnCount = 0;

std::vector<Model> preloadedModels;
std::vector<glm::vec3> preloadedModelLocations;
int preloadedCount = 0;
Model tempBackpackModel;

Model tempImpModel;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

int floorTextureIndex;
int roofTextureIndex;
int wallTextureIndex;
int doorTextureIndex = 6;



bool roofOn;
float torchValue;
bool directionalActive;
float cubeVerticies[1000]{};
float skyboxVertices[1000]{};
unsigned int VBO, cubeVAO;
unsigned int lightCubeVAO;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int initialTime = time(NULL), finalTime, frameCount = 0;
float currentFPS = 0;

// lighting
//glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int console = 0;
std::string command;
float modelPosX;
float modelPosY;
float modelPosZ;
bool loadModel = false;
Model commandLoadedModel("");
int numTriangles;
//Shader modelShader("model_loading.vert", "model_loading.frag");
//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
//glm::mat4 view = camera.GetViewMatrix();



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);
    //(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    // build and compile our shader zprogram
    // ------------------------------------
    Shader lightingShader("multiple_lights.vert", "multiple_lights.frag");
    Shader lightCubeShader("light_cube.vert", "light_cube.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");
    Shader modelShader("model_loading.vert", "model_loading.frag");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    //Verticies made from text file
    
    loadCubeVerticies();

    //loadRoomData();
    loadSettings();

    //loading skybox
    loadSkyboxVertices();

    Shader newModelShader("model_loading.vert", "model_loading.frag");
    
    

    //Model impModel("Models/woodImp/woodImp.obj");
    //Model backpackModel("Models/backpack/backpack.obj");

    unsigned int diffuseMap = loadTexture("Textures/diffuse/woodBox.png");
    unsigned int specularMap = loadTexture("Textures/woodBox_specular.png");
    
    unsigned int textures[10];
    textures[0] = loadTexture("Textures/textures/floor1.jpg");
    textures[1] = loadTexture("Textures/textures/floor2.jpg");
    textures[2] = loadTexture("Textures/textures/floor3.jpg");
    textures[3] = loadTexture("Textures/textures/floor4.jpg");
    textures[4] = loadTexture("Textures/textures/floor5.jpg");
    textures[5] = loadTexture("Textures/textures/wall2.jpg");
    textures[6] = loadTexture("Textures/textures/wall3.jpg");
    textures[7] = loadTexture("Textures/textures/wall4.jpg");
    textures[8] = loadTexture("Textures/textures/wall5.jpg");
    textures[9] = loadTexture("Textures/textures/wall6.jpg");

    // skybox VAO-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    unsigned int cubemapTexture = loadCubemap(skyboxFaces);
    stbi_set_flip_vertically_on_load(true);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
    /*lightingShader.use();
    lightingShader.setInt("material.diffuse", 0);
    lightingShader.setInt("material.specular", 1);*/

    //unsigned int cubemapTexture = loadCubemap(faces);

    //Opening Message
    std::cout << "<< To activate console: press '1' >>" << std::endl;
    std::cout << "Avaliable commands are:" << std::endl;
    std::cout << "fps" << std::endl;
    std::cout << "load level 'filename.txt'" << std::endl;
    std::cout << "load settings 'filename.txt'" << std::endl;
    std::cout << "spawn 'filename.obj' 'x' 'y' 'z'" << std::endl;
    std::cout << "triangles" << std::endl;
    std::cout << "help" << std::endl;
    std::cout << "<< Note that all commands are in lower-case and don't require an apostrophe(') before or after the parameters>>" << std::endl;
    std::cout << "<< DEFAULT LEVEL FILE NAME = roomInfo.txt >>" << std::endl;
    std::cout << "<< DEFAULT SETTINGS FILE NAME = levelOptions.txt >>" << std::endl;
    std::cout << "<< BACKPACK SPAWN PARAMETER = 'backpack/backpack.obj' >>" << std::endl;
    std::cout << "" << std::endl;
    
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 pointLightColors[] = {
        glm::vec3(1.0f, 0.6f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0, 0.0),
        glm::vec3(0.2f, 0.2f, 1.0f)
        };

        //lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        //lightPos.y = sin(glfwGetTime() / 2.0f) * 1.0f;

        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);
        lightingShader.setFloat("material.shininess", 32.0f);


        // light properties
        glm::vec3 lightColor;
        lightColor.x = sin(glfwGetTime() * 2.0f);
        lightColor.y = 0.5f;
        lightColor.z = 0.5f;
        // directional light
        if (directionalActive) {
            lightingShader.setVec3("dirLight.direction", sin(glfwGetTime() * 0.2f), sin(glfwGetTime() * 0.2f), -0.3f);
            lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.1f);
            lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.7f);
            lightingShader.setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);
        }
        else {
            lightingShader.setVec3("dirLight.direction", sin(glfwGetTime() * 2.0f), -1.0f, -0.3f);
            lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.1f);
            lightingShader.setVec3("dirLight.diffuse", 0.0f, 0.0f, 0.0f);
            lightingShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);
        }

        // point light 1

        for (unsigned int i = 0; i < pointLights.size();i++) {
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLights[i]);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.05f, 0.05f, 0.05f);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 0.1f, 0.1f, 0.1f);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].specular", 0.6f, 0.6f, 0.6f);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.027);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.0028);


        }for (unsigned int i = pointLights.size(); i < 20;i++) {
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].position", glm::vec3(0.0f));
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].ambient", 0.00f, 0.00f, 0.00f);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].diffuse", 0.0f, 0.0f, 0.0f);
            lightingShader.setVec3("pointLights[" + std::to_string(i) + "].specular", 0.0f, 0.0f, 0.0f);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].constant", 1.0f);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].linear", 0.027);
            lightingShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", 0.0028);
        }

        lightingShader.setVec3("spotLights[0].position", camera.Position);
        lightingShader.setVec3("spotLights[0].direction", camera.Front);
        lightingShader.setVec3("spotLights[0].ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLights[0].diffuse", torchValue, torchValue, torchValue);
        lightingShader.setVec3("spotLights[0].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLights[0].constant", 1.0f);
        lightingShader.setFloat("spotLights[0].linear", 0.09);
        lightingShader.setFloat("spotLights[0].quadratic", 0.032);
        lightingShader.setFloat("spotLights[0].cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLights[0].outerCutOff", glm::cos(glm::radians(15.0f)));
        // spotLight on camera


        // spotLight facing down
        /*lightingShader.setVec3("spotLights[1].position", 0.0f,-6.0f,0.0f);
        lightingShader.setVec3("spotLights[1].direction", 0.1f, -1.0f, 0.1f);
        lightingShader.setVec3("spotLights[1].ambient", 0.0f, 0.0f, 0.0f);
        lightingShader.setVec3("spotLights[1].diffuse", 1.0f, 1.0f, 1.0f);
        lightingShader.setVec3("spotLights[1].specular", 1.0f, 1.0f, 1.0f);
        lightingShader.setFloat("spotLights[1].constant", 1.0f);
        lightingShader.setFloat("spotLights[1].linear", 0.09);
        lightingShader.setFloat("spotLights[1].quadratic", 0.032);
        lightingShader.setFloat("spotLights[1].cutOff", glm::cos(glm::radians(12.5f)));
        lightingShader.setFloat("spotLights[1].outerCutOff", glm::cos(glm::radians(17.0f)));*/

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);


        //binding diffuse map texture
        /*glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseMap);*/

        //binding specular map texture
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, specularMap);
        // render the cube
        /*glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);*/
        glBindVertexArray(cubeVAO);
        //for (unsigned int i = 0; i < 10; i++)
        //{
        //    glActiveTexture(GL_TEXTURE0);
        //    glBindTexture(GL_TEXTURE_2D, textures[i]);
        //    // calculate the model matrix for each object and pass it to shader before drawing
        //    glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        //    //model = glm::translate(model, glm::vec3(-10.0f, 0.0f, -1.0f * i));
        //    model = glm::translate(model, glm::vec3(-10.0f, 0.0f, -1.0f * i));
        //    //float angle = 20.0f * i;
        //    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //    model = glm::rotate(model, glm::radians((float)glfwGetTime() * 20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //    //model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        //    lightingShader.setMat4("model", model);

        //    glDrawArrays(GL_TRIANGLES, 0, 36);
        //}
        //Floors 
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[floorTextureIndex]);
        for (unsigned int i = 0; i < floorNoRoof.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, floorNoRoof[i]);
            //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }

        //ROOFS
        if (roofOn) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[roofTextureIndex]);
            for (unsigned int i = 0; i < floorWithRoof.size();i++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, floorWithRoof[i]);
                //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                lightingShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);
                countTrangles(36);
            }
        }


        //Walls
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[wallTextureIndex]);
        //Normal Walls
        //CORNER WALLS
        for (unsigned int i = 0; i < cornerWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cornerWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }for (unsigned int i = 0; i < cornerWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cornerWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }
        //EAST WALLS       
        for (unsigned int i = 0; i < eastWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, eastWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }
        //NORTH WALLS
        for (unsigned int i = 0; i < northWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, northWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }
        //Small Walls
        //CORNER SMALL WALLS
        for (unsigned int i = 0; i < cornerSmallWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cornerSmallWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }for (unsigned int i = 0; i < cornerSmallWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cornerSmallWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }
        //EAST  SMALL WALLS       
        for (unsigned int i = 0; i < eastSmallWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, eastSmallWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }
        //NORTH SMALL WALLS
        for (unsigned int i = 0; i < northSmallWalls.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, northSmallWalls[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }

        //DOORS
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[doorTextureIndex]);

        //EAST DOORS
        for (unsigned int i = 0; i < eastDoors.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, eastDoors[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            //model = glm::rotate(model, glm::radians(90.0f),glm::vec3(0.0f,1.0f,0.0f));
            model = glm::scale(model, glm::vec3(1.0f, 0.2f, 0.5f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }
        //NORTH DOORS
        for (unsigned int i = 0; i < northDoors.size();i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, northDoors[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(1.0f, 0.2f, 0.5f));
            lightingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }

        // also draw the lamp object(s)
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);



        // we now draw as many light bulbs as we have point lights.
        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < pointLights.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLights[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
            lightCubeShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            countTrangles(36);
        }

        //lightingShader.use();
        //lightingShader.setMat4("projection", projection);
        //lightingShader.setMat4("view", view);
        //// render the loaded model
        //glm::mat4 loadedModel = glm::mat4(1.0f);
        //loadedModel = glm::translate(loadedModel, glm::vec3(0.0f, 2.0f, 0.0f)); // translate it down so it's at the center of the scene
        ////loadedModel = glm::scale(loadedModel, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        //loadedModel = glm::scale(loadedModel, glm::vec3(0.001f, 0.001f, 0.001f));	// it's a bit too big for our scene, so scale it down
        //lightingShader.setMat4("model", loadedModel);
        //ourModel.Draw(lightingShader);


        //if (loadModel) {
        //    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //    glm::mat4 view = camera.GetViewMatrix();
        //    lightingShader.use();
        //    lightingShader.setMat4("projection", projection);
        //    lightingShader.setMat4("view", view);
        //    glm::mat4 model = glm::mat4(1.0f);

        //    model = glm::translate(model, glm::vec3(modelPosX, modelPosY, modelPosZ));
        //    model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        //    lightingShader.setMat4("model", model);
        //    //std::cout << "Spawning " << commands[1] << " at:" << "(" << commands[2] << "," << commands[3] << "," << commands[4] << ")" << std::endl;
        //    commandLoadedModel.Draw(lightingShader);
        //}

        for (int i = 0; i < preloadedModels.size(); i++)
        {
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            lightingShader.use();
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, preloadedModelLocations[i]);
            model = glm::scale(model, preloadedScaleValues[i]);
            lightingShader.setMat4("model", model);
            //std::cout << "Spawning " << commands[1] << " at:" << "(" << commands[2] << "," << commands[3] << "," << commands[4] << ")" << std::endl;
            preloadedModels[i].Draw(lightingShader);
        }





        for (int i = 0; i < spawnedModels.size(); i++)
        {
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            lightingShader.use();
            lightingShader.setMat4("projection", projection);
            lightingShader.setMat4("view", view);
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, spawnLocations[i]);
            model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
            lightingShader.setMat4("model", model);
            //std::cout << "Spawning " << commands[1] << " at:" << "(" << commands[2] << "," << commands[3] << "," << commands[4] << ")" << std::endl;
            spawnedModels[i].Draw(lightingShader);
        }

        //--- SKYBOX ---
        // draw skybox as last
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
         //projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //skyboxShader.setMat4("model", model);
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        countTrangles(36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default
        //--- END SKYBOX ---
        

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);

        //FPS logic after swapping buffers (loading a new frame)
        ++frameCount;
        finalTime = time(NULL);
        if (finalTime - initialTime >= 1) 
        {
            //std::cout << "FPS is: " << frameCount / (finalTime - initialTime) << std::endl;
            currentFPS = frameCount / (finalTime - initialTime);
            frameCount = 0;
            initialTime = finalTime;
            finalTime = 0;
        }

        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (!console)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            std::cout << "<< Console mode ACTIVATED >>" << std::endl;
            console = 1;
            glfwSetCharCallback(window, character_callback);
        }
    }
    else if (console)
    {
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            excecuteCommand(parse(command));
            console = 0;
            command = "";
            std::cout << "<< Console mode DEACTIVATED >>" << std::endl;
            std::cout << "<< To activate console: press '1' >>" << std::endl;
            std::cout << "" << std::endl;
            glfwSetCharCallback(window, NULL);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
    {
        camera.ToggleHeightLock();
    }
}



void loadRoomData() {
    /*
    E = East Pointing wall
    N = North Point wall
    L = Point Light
    ^ = Easting Point Door
    < = North Point Door
    * = Floor without roof
    & = Floor with roof
    */

    int xPosCount = -10;
    std::string filename = "Config/roomInfo.txt";
    std::ifstream in(filename, std::ios::out);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open settings file "" << filename << "" for reading!" << std::endl;
       
    }
    int zPosCount = -10;
    std::string line;
    while (std::getline(in, line)) {
        std::string delimiter = ",";
        size_t pos = 0;
        char prefabCode; // define a string variable
        zPosCount++;
        // use find() function to get the position of the delimiters
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            prefabCode = line.substr(0, pos)[0]; // store the substring
            
            switch (prefabCode)
            {
            case 'C':
                cornerWalls.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                break;
            case 'E':
                eastWalls.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                break;
            case 'N':
                northWalls.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                break;
            case 'L':
                pointLights.push_back(glm::vec3(xPosCount, -0.6f, zPosCount));
                break;
            case '^':
                eastDoors.push_back(glm::vec3(xPosCount, -0.6f, zPosCount));
                break;
            case '<':
                northDoors.push_back(glm::vec3(xPosCount, -0.6f, zPosCount));
                break;
            case 'G':
                commandLoadedModel.Clear();
                commandLoadedModel.loadModel("Models/woodImp/woodImp.obj");
                preloadedModels.push_back(commandLoadedModel);
                preloadedModelLocations.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                //impSpawns.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                break;
            case 'B':
                commandLoadedModel.Clear();
                commandLoadedModel.loadModel("Models/backpack/backpack.obj");
                preloadedModels.push_back(commandLoadedModel);
                preloadedModelLocations.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                //backpackSpawns.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                break;
            default:
                
                break;
            }
            floorNoRoof.push_back(glm::vec3(xPosCount, -1.7f, zPosCount));

            if (prefabCode != '*') {
                floorWithRoof.push_back(glm::vec3(xPosCount, -0.3f, zPosCount));
            }
            
            //vertices[count] = std::stof(vertex);
            xPosCount++;
            std::cout << prefabCode << std::endl;
            line.erase(0, pos + delimiter.length());  /* erase() function store the current positon and move to next token. */
        }
        xPosCount = -10;
    }
    in.close();
}

void loadSettings() {

    int xPosCount = 1;
    std::string filename = "Config/levelOptions.txt";
    std::ifstream in(filename, std::ios::out);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open settings file "" << filename << "" for reading!" << std::endl;

    }
    int zPosCount = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::string delimiter = ",";
        size_t pos = 0;
        std::string textureCode; // define a string variable
        zPosCount++;
        // use find() function to get the position of the delimiters
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            textureCode = line.substr(0, pos)[pos-1]; 
            if (xPosCount == 1) {//Floor Texture
                floorTextureIndex = std::stoi(textureCode);
                roofTextureIndex = std::stoi(textureCode);
            }
            if (xPosCount == 2) {//Floor Texture
                wallTextureIndex = std::stoi(textureCode);
            }
            if (xPosCount == 3) {
                if (std::stoi(textureCode) == 0) {
                    roofOn = false;
                }
                else {
                    roofOn = true;
                }
            }if (xPosCount == 4) {
                if (std::stoi(textureCode) == 0) {
                    torchValue = 0.0f;
                }
                else {
                    torchValue = 1.0f;
                }
            }if (xPosCount == 5) {
                if (std::stoi(textureCode) == 0) {
                    directionalActive = false;
                }
                else {
                    directionalActive = true;
                }
            }if (xPosCount == 6) { //skybox options
                //std::string skyboxCode = line.substr(0, pos)[pos - 1];
                std::cout << textureCode << std::endl;
                loadSkyboxFaces(textureCode);
            }
            

            //vertices[count] = std::stof(vertex);
            xPosCount++;
            std::cout << xPosCount << std::endl;
            std::cout << textureCode << std::endl;
            line.erase(0, pos + delimiter.length());  /* erase() function store the current positon and move to next token. */
        }
    }
    in.close();
}

void loadSettingsFromConsole(string path) {

    int xPosCount = 1;
    //std::string filename = "levelOptions.txt";
    std::ifstream in("Config/"+path, std::ios::out);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open settings file " << path << " for reading!" << std::endl;

    }
    int zPosCount = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::string delimiter = ",";
        size_t pos = 0;
        std::string textureCode; // define a string variable
        zPosCount++;
        // use find() function to get the position of the delimiters
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            textureCode = line.substr(0, pos)[pos - 1];
            if (xPosCount == 1) {//Floor Texture
                floorTextureIndex = std::stoi(textureCode);
                roofTextureIndex = std::stoi(textureCode);
            }
            if (xPosCount == 2) {//Floor Texture
                wallTextureIndex = std::stoi(textureCode);
            }
            if (xPosCount == 3) {
                if (std::stoi(textureCode) == 0) {
                    roofOn = false;
                }
                else {
                    roofOn = true;
                }
            }if (xPosCount == 4) {
                if (std::stoi(textureCode) == 0) {
                    torchValue = 0.0f;
                }
                else {
                    torchValue = 1.0f;
                }
            }if (xPosCount == 5) {
                if (std::stoi(textureCode) == 0) {
                    directionalActive = false;
                }
                else {
                    directionalActive = true;
                }
            }if (xPosCount == 6) { //skybox options
                //std::string skyboxCode = line.substr(0, pos)[pos - 1];
                std::cout << textureCode << std::endl;
                loadSkyboxFaces(textureCode);
            }


            //vertices[count] = std::stof(vertex);
            xPosCount++;
            line.erase(0, pos + delimiter.length());  /* erase() function store the current positon and move to next token. */
        }
    }
    in.close();
}

void loadRoomDataFromConsole(string path) {
    /*
    E = East Pointing wall
    N = North Point wall
    L = Point Light
    ^ = Easting Point Door
    < = North Point Door
    * = Floor without roof
    & = Floor with roof
    */

    int xPosCount = -10;
    //std::string filename = "roomInfo.txt";
    std::ifstream in("Config/"+path, std::ios::out);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open settings file "" << filename << "" for reading!" << std::endl;

    }
    int zPosCount = -10;
    std::string line;
    while (std::getline(in, line)) {
        std::string delimiter = ",";
        size_t pos = 0;
        char prefabCode; // define a string variable
        zPosCount++;
        // use find() function to get the position of the delimiters
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            prefabCode = line.substr(0, pos)[0]; // store the substring

            switch (prefabCode)
            {
            case 'C':
                cornerWalls.push_back(glm::vec3(xPosCount, 0.2f, zPosCount));
                break;
            case 'E':
                eastWalls.push_back(glm::vec3(xPosCount, 0.2f, zPosCount));
                break;
            case 'N':
                northWalls.push_back(glm::vec3(xPosCount, 0.2f, zPosCount));
                break;
            case 'c':
                cornerSmallWalls.push_back(glm::vec3(xPosCount, -0.4, zPosCount));
                break;
            case 'e':
                eastSmallWalls.push_back(glm::vec3(xPosCount, -0.4f, zPosCount));
                break;
            case 'n':
                northSmallWalls.push_back(glm::vec3(xPosCount, -0.4f, zPosCount));
                break;
            case 'L':
                pointLights.push_back(glm::vec3(xPosCount, 0.6f, zPosCount));
                break;
            case '^':
                eastDoors.push_back(glm::vec3(xPosCount, 0.6f, zPosCount));
                break;
            case '<':
                northDoors.push_back(glm::vec3(xPosCount, 0.6f, zPosCount));
                break;
            case 'G':
                tempImpModel.Clear();
                tempImpModel.loadModel("Models/woodImp/woodImp.obj");
                preloadedModels.push_back(tempImpModel);
                preloadedModels[preloadedCount].Clear();
                preloadedModels[preloadedCount].loadModel("Models/woodImp/woodImp.obj");
                preloadedModelLocations.push_back(glm::vec3(xPosCount, 0.0f, zPosCount));
                preloadedScaleValues.push_back(glm::vec3(0.5f, 0.5f, 0.5f));
                //impSpawns.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                preloadedCount++;
                break;
            case 'B':
                tempBackpackModel.Clear();
                tempBackpackModel.loadModel("Models/backpack/backpack.obj");
                preloadedModels.push_back(tempBackpackModel);
                preloadedModels[preloadedCount].Clear();
                preloadedModels[preloadedCount].loadModel("Models/backpack/backpack.obj");
                preloadedModelLocations.push_back(glm::vec3(xPosCount, 0.0f, zPosCount));
                preloadedScaleValues.push_back(glm::vec3(0.1f, 0.1f, 0.1f));
                //backpackSpawns.push_back(glm::vec3(xPosCount, -1.0f, zPosCount));
                preloadedCount++;
                break;
            default:

                break;
            }
            if (prefabCode != '*') {
                floorNoRoof.push_back(glm::vec3(xPosCount, -0.5f, zPosCount));
            }
            

            if (prefabCode != '*' && prefabCode != 'e' && prefabCode != 'n' && prefabCode != 'c' && prefabCode != '#') {
                floorWithRoof.push_back(glm::vec3(xPosCount, 0.9f, zPosCount));
            }

            //vertices[count] = std::stof(vertex);
            xPosCount++;
            line.erase(0, pos + delimiter.length());  /* erase() function store the current positon and move to next token. */
        }
        xPosCount = -10;
    }
    in.close();
    std::cout << path << " loaded successfully" << std::endl;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    std::cout << (char)codepoint;
    command += (char)codepoint;
}

void excecuteCommand(std::vector<std::string>commands)
{
    std::cout << std::endl;
    if (commands[0] == "spawn")
    {
        if (commands.size() > 4)
        {
            if (commands[2] != "" && commands[3] != "" && commands[4] != "") {
                Model tempModel;
                //commandLoadedModel.Clear();

                if (commands[1] == "backpack") {
                    tempModel.Clear();
                    tempModel.loadModel("Models/backpack/backpack.obj");
                    spawnedModels.push_back(tempModel);
                    spawnedModels[spawnCount].loadModel("Models/backpack/backpack.obj");
                }
                else if (commands[1] == "imp") {
                    tempModel.Clear();
                    tempModel.loadModel("Models/woodImp/woodImp.obj");
                    spawnedModels.push_back(tempModel);
                    spawnedModels[spawnCount].loadModel("Models/woodImp/woodImp.obj");
                }
                else {
                    tempModel.Clear();
                    tempModel.loadModel("Models/" + commands[1]);
                    spawnedModels.push_back(tempModel);
                    spawnedModels[spawnCount].loadModel("Models/" + commands[1]);
                    
                }

                modelPosX = std::stof(commands[2] + ".0f");
                modelPosY = std::stof(commands[3] + ".0f");
                modelPosZ = std::stof(commands[4] + ".0f");
                spawnLocations.push_back(glm::vec3(modelPosX, modelPosY, modelPosZ));
                loadModel = true;
                countTrangles(spawnedModels[spawnCount].GetNumVerticies(0));
                spawnCount++;
            }
            else
            {
                std::cout << "Please enter the 3 spawn LOCATION CO-ORDINATES values (x y z)" << std::endl;
            }
        }
        else
        {
            std::cout << "Please enter the MODEL NAME and LOCATION CO-ORDINATES for the model you wish to spawn" << std::endl;
        }
    }
    else if (commands[0] == "fps")  //boost::iequals(commands[0], "fps") - attempt to ignore case sensitivity.
    {
        std::cout << "Current FPS is: " << currentFPS << std::endl;
    }
    else if (commands[0] == "triangles")
    {
        std::cout << "Current number of TRIANGLES is: " << numTriangles << std::endl;
    }
    else if (commands[0] == "load")
    {
        if (commands[1] == "level")
        {
            if (commands[2] != "")
            {
                std::cout << "Loading level file: " << commands[2] << std::endl;
                loadRoomDataFromConsole(commands[2]);
            }
            else
            {
                std::cout << "Enter a level filename to load" << std::endl;
            }

        }
        else if (commands[1] == "settings")
        {
            if (commands[2] != "")
            {
                std::cout << "Loading settings file: " << commands[2] << std::endl;
                loadSettingsFromConsole(commands[2]);
            }
            else
            {
                std::cout << "Enter a settings filename to load" << std::endl;
            }
        }
        else
        {
            std::cout << "Unknown 'load' command. Try either 'load level (filename)' OR ' load settings (filename)'" << std::endl;
        }

    }
    else if (commands[0] == "help")
    {
        //Additional help output to explain how engine works
        std::cout << "HELP: ENGINE FUNCTIONALITY IS AS FOLLOWS " << std::endl;
        std::cout << "To move character/camera: WASD keys / SPACE for Up / LEFTSHIFT for Down" << std::endl;
        std::cout << "To move look around scene: move mouse" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "Avaliable commands are:" << std::endl;
        std::cout << "fps" << std::endl;
        std::cout << "load level 'filename.txt'" << std::endl;
        std::cout << "load settings 'filename.txt'" << std::endl;
        std::cout << "spawn 'filename.obj' 'x' 'y' 'z'" << std::endl;
        std::cout << "triangles" << std::endl;
        std::cout << "help" << std::endl;
        std::cout << "<< Note that all commands are in lower-case and don't require an apostrophe(') before or after the parameters>>" << std::endl;
        std::cout << "<< DEFAULT LEVEL FILE NAME = roomInfo.txt >>" << std::endl;
        std::cout << "<< DEFAULT SETTINGS FILE NAME = levelOptions.txt >>" << std::endl;
        std::cout << "<< BACKPACK SPAWN PARAMETER = 'backpack/backpack.obj' >>" << std::endl;
    }
    else
    {
        std::cout << "Unknown command" << std::endl;
        std::cout << "Avaliable commands are:" << std::endl;
        std::cout << "fps" << std::endl;
        std::cout << "load level 'filename.txt'" << std::endl;
        std::cout << "load settings 'filename.txt'" << std::endl;
        std::cout << "spawn 'filename.obj' 'x' 'y' 'z'" << std::endl;
        std::cout << "triangles" << std::endl;
        std::cout << "help" << std::endl;
        std::cout << "<< Note that all commands are in lower-case and don't require an apostrophe(') before or after the parameters>>" << std::endl;
        std::cout << "<< DEFAULT LEVEL FILE NAME = roomInfo.txt >>" << std::endl;
        std::cout << "<< DEFAULT SETTINGS FILE NAME = levelOptions.txt >>" << std::endl;
        std::cout << "<< BACKPACK SPAWN PARAMETER = 'backpack/backpack.obj' >>" << std::endl;
    }
}


std::vector<std::string> parse(std::string s)
{
    s += " ";
    std::vector<std::string>words;
    std::string word;
    for (auto c : s)
    {
        if (c == ' ')
        {
            words.push_back(word);
            word = "";
        }
        else
        {
            word += c;
        }
    }

    return words;
}

Model newModel()
{
    return Model();
}

void countTrangles(int addedNum) {
    numTriangles += (addedNum / 3);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void loadCubeVerticies() {
    int count = 0;
    std::string filename = "Config/cubeVerticesInfo.txt";
    std::ifstream in(filename, std::ios::out);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open settings file "" << filename << "" for reading!" << std::endl;        
    }
    int lineCount = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::string delimiter = ",";
        size_t pos = 0;
        std::string vertex; // define a string variable
        lineCount++;
        // use find() function to get the position of the delimiters
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            vertex = line.substr(0, pos); // store the substring
            cubeVerticies[count] = std::stof(vertex);
            count++;
            std::cout << vertex << std::endl;
            line.erase(0, pos + delimiter.length());  /* erase() function store the current positon and move to next token. */
        }
    }std::cout << count << std::endl;
    in.close();

    // first, configure the cube's VAO (and VBO)
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerticies), cubeVerticies, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
void loadSkyboxVertices() {
    int count = 0;
    std::string filename = "Config/skyboxVerticesInfo.txt";
    std::ifstream in(filename, std::ios::out);
    if (!in.is_open()) {
        std::cerr << "Error: Unable to open settings file "" << filename << "" for reading!" << std::endl;
    }
    int lineCount = 0;
    std::string line;
    while (std::getline(in, line)) {
        std::string delimiter = ",";
        size_t pos = 0;
        std::string vertex; // define a string variable
        lineCount++;
        // use find() function to get the position of the delimiters
        while ((pos = line.find(delimiter)) != std::string::npos)
        {
            vertex = line.substr(0, pos); // store the substring
            skyboxVertices[count] = std::stof(vertex);
            count++;
            std::cout << vertex << std::endl;
            line.erase(0, pos + delimiter.length());  /* erase() function store the current positon and move to next token. */
        }
    }std::cout << count << std::endl;
    in.close();

    // first, configure the cube's VAO (and VBO)
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVerticies), cubeVerticies, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)

    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            stbi_set_flip_vertically_on_load(false);
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
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

void loadSkyboxFaces(string folder) {

    if (folder == "F") {
        folder = "Default";
    }
    else if (folder == "S") {
        folder = "Sunset";
    }
    else if (folder == "D") {
        folder = "Daytime";
    }
    else if (folder == "P") {
        folder = "Space";
    }
    else if (folder == "M") {
        folder = "Midnight";
    }

    skyboxFaces.push_back("Skybox/" + folder + "/right.jpg");
    skyboxFaces.push_back("Skybox/" + folder + "/left.jpg");
    skyboxFaces.push_back("Skybox/" + folder + "/top.jpg");
    skyboxFaces.push_back("Skybox/" + folder + "/bottom.jpg");
    skyboxFaces.push_back("Skybox/" + folder + "/front.jpg");
    skyboxFaces.push_back("Skybox/" + folder + "/back.jpg");
}
