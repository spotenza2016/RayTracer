#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "RayTracer.h"
using namespace std;
// Global Ray Tracer Settings
float highQualitySize = 256;
float lowQualitySize = 32;
float highQualityProjDistance = 144.0f;
float lowQualityProjDistance = 18.0f;
float stepSize = 4;
float thetaSize = 10;
float cursorSens = 0.1f;
bool printLocation = false;
// Global Ray Tracer Variables
RayTracer rayTracer;
vector<float> camera = {100.0f, 100.0f, 0.0f};
vector<float> lookAtVec = {0.0f, 0.0f, -1.0f};
vector<float> upVec = {0.0f, 1.0f, 0.0f};
float yaw = 0.0f;
float pitch = 0.0f;
float roll = 0.0f;
double lastCursorPosX;
double lastCursorPosY;
bool render = true;
bool cameraControlsEnabled = false;
bool record = false;
vector<vector<vector<float>>> movements;
// Function that processes keyboard inputs
void processInput(GLFWwindow *window)
{
    // Escape closes window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // Switch to Orthogonal with O
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        rayTracer.orthogonal = true;
        render = true;
    }
    // Switch to Perspective with P
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        rayTracer.orthogonal = false;
        render = true;
    }
    if (cameraControlsEnabled) {
        // Mouse Camera controls
        double cursorX, cursorY;
        glfwGetCursorPos(window, &cursorX, &cursorY);
        if (cursorX != lastCursorPosX || cursorY != lastCursorPosY) {
            yaw += (float)(cursorX - lastCursorPosX) * cursorSens;
            pitch += (float)(cursorY - lastCursorPosY) * cursorSens;
            lastCursorPosY = cursorY;
            lastCursorPosX = cursorX;
            render = true;
        }
        // Move camera left with A
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            // Moves camera in direction of -u
            camera = RayTracer::addVec(camera, RayTracer::scalarVec(-1 * stepSize,
                                                                    RayTracer::normalizeVec(RayTracer::crossVec(upVec, RayTracer::scalarVec(-1,
                                                                                                                                            lookAtVec)))));
            render = true;
        }
        // Move camera right with D
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            // Moves camera in direction of u
            camera = RayTracer::addVec(camera, RayTracer::scalarVec(stepSize,
                                                                    RayTracer::normalizeVec(RayTracer::crossVec(upVec, RayTracer::scalarVec(-1,
                                                                                                                                            lookAtVec)))));
            render = true;
        }
        // Move camera forwards with W
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            // Moves camera in direction of -w
            camera = RayTracer::addVec(camera, RayTracer::scalarVec(stepSize,
                                                                    RayTracer::normalizeVec(lookAtVec)));
            render = true;
        }
        // Move camera backwards with S
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            // Moves camera in direction of w
            camera = RayTracer::addVec(camera, RayTracer::scalarVec(-1 * stepSize,
                                                                    RayTracer::normalizeVec(lookAtVec)));
            render = true;
        }
        // Move camera up with Space
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            // Moves camera in direction of v
            camera = RayTracer::addVec(camera, RayTracer::scalarVec(stepSize,
                                                                    RayTracer::normalizeVec(upVec)));
            render = true;
        }
        // Move camera up with Left Control
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            // Moves camera in direction of -v
            camera = RayTracer::addVec(camera, RayTracer::scalarVec(-1 * stepSize,
                                                                    RayTracer::normalizeVec(upVec)));
            render = true;
        }
        // Turn camera left with Left Arrow
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            yaw -= thetaSize;
            render = true;
        }
        // Turn camera right with Right Arrow
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            yaw += thetaSize;
            render = true;
        }
        // Turn camera up with Up Arrow
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            pitch -= thetaSize;
            render = true;
        }
        // Turn camera down with Down Arrow
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            pitch += thetaSize;
            render = true;
        }
        // Not implemented: Roll camera counter clockwise with Q
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            roll -= thetaSize;
            render = true;
        }
        // Not implemented: Roll camera clockwise with E
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            roll += thetaSize;
            render = true;
        }
    }
    // Press H to switch to high quality resolution
    if (glfwGetKey(window, GLFW_KEY_H)) {
        rayTracer.imgSizeX = highQualitySize;
        rayTracer.imgSizeY = highQualitySize;
        rayTracer.projectionDistance = highQualityProjDistance;
        render = true;
        cameraControlsEnabled = false;
    }
    // Press L to switch to low quality resolution
    if (glfwGetKey(window, GLFW_KEY_L)) {
        rayTracer.imgSizeX = lowQualitySize;
        rayTracer.imgSizeY = lowQualitySize;
        rayTracer.projectionDistance = lowQualityProjDistance;
        render = true;
        cameraControlsEnabled = true;
        glfwGetCursorPos(window, &lastCursorPosX, &lastCursorPosY);
    }
    // Press U to turn off light visualization
    if (glfwGetKey(window, GLFW_KEY_U)) {
        render = true;
        rayTracer.lightVisualization = false;
    }
    // Press V to turn on light visualization
    if (glfwGetKey(window, GLFW_KEY_I)) {
        render = true;
        rayTracer.lightVisualization = true;
    }
    // Press R to start recording movements
    if (glfwGetKey(window, GLFW_KEY_R)) {
        render = true;
        record = true;
    }
    // Press T to save all movements as PPM files
    if (glfwGetKey(window, GLFW_KEY_T)) {
        record = false;
        rayTracer.imgSizeX = highQualitySize;
        rayTracer.imgSizeY = highQualitySize;
        rayTracer.projectionDistance = highQualityProjDistance;
        cameraControlsEnabled = false;
        // Renders each frame (program will say not responding while this occurs)
        for (int i = 0; i < movements.size(); i++) {
            unsigned char* move = rayTracer.produceImage(movements.at(i)[0],
                                                         movements.at(i)[1], movements.at(i)[2]);
            // To render: ffmpeg -framerate 20 -i rayTrace%d.ppm -c:v libx264 -crf 25 -vf "scale=256:256,format=yuv420p" -movflags +faststart rayTrace.mp4
            rayTracer.takePicture("rayTrace" + to_string(i) + ".ppm", move);
        }
        movements = vector<vector<vector<float>>>();
        render = true;
    }
    // If we rendered, transform our vectors based on pitch yaw and roll
    if (render) {
        lookAtVec = RayTracer::transformVector({0.0f, 0.0f, -1.0f}, pitch, yaw,
                                               roll);
        upVec = RayTracer::transformVector({0.0f, 1.0f, 0.0f}, pitch, yaw, roll);
    }
}
// Given by professor
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;
const char *vertexShaderSource = "#version 330 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "layout (location = 1) in vec3 aColor;\n"
                                 "layout (location = 2) in vec2 aTexCoord;\n"
                                 "out vec3 ourColor;\n"
                                 "out vec2 TexCoord;\n"
                                 "void main()\n"
                                 "{\n"
                                 "gl_Position = vec4(aPos, 1.0);\n"
                                 "ourColor = aColor;\n"
                                 "TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "in vec3 ourColor;\n"
                                   "in vec2 TexCoord;\n"
                                   "uniform sampler2D texture1;\n"
                                   "void main()\n"
                                   "{\n"
                                   " FragColor = texture(texture1, TexCoord);\n"
                                   "}\n\0";
// Callback for if errors occur
void error_callback(int error, const char* log) {
    cout << "Error: " << error << " " << log << endl;
}
// Function provided by professor:
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
// Main Function
int main() {
    // Code for GLFW/GLEW Setup and 2D Array Display given by professor
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwSetErrorCallback(error_callback);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Ray Tracer",
                                          nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    int glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        glfwTerminate();
        return -1;
    }
    // build and compile the shaders
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog <<
                  std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog <<
                  std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog <<
                  std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
            // positions // colors // texture coords
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3 // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                 GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 *
                                                                               sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 *
                                                                               sizeof(float)));
    glEnableVertexAttribArray(2);
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Hide cursor for mouse support
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(window, &lastCursorPosX, &lastCursorPosY);
    rayTracer.imgSizeX = highQualitySize;
    rayTracer.imgSizeY = highQualitySize;
    rayTracer.projectionDistance = highQualityProjDistance;
    // Render Loop
    while(!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        if (render) {
            if (record) {
                movements.push_back({camera, lookAtVec, upVec});
            }
            if (printLocation) {
                cout << "Camera: " << camera[0] << " " << camera[1] << " " <<
                     camera[2] << endl;
                cout << "LookAtVec: " << lookAtVec[0] << " " << lookAtVec[1] << " "
                     << lookAtVec[2] << endl;
                cout << "UpVec: " << upVec[0] << " " << upVec[1] << " " << upVec[2]
                     << endl;
            }
            unsigned char* currImage = rayTracer.produceImage(camera, lookAtVec,
                                                              upVec);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rayTracer.imgSizeX,
                         rayTracer.imgSizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, currImage);
            glGenerateMipmap(GL_TEXTURE_2D);
            render = false;
        }
        // render container
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}