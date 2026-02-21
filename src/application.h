#pragma once

#include <optional>
#include <vector>
#include <GLFW/glfw3.h>

#include "render/camera.h"
#include "render/shader.h"
#include "systems/input_system.h"

constexpr unsigned int SCALE = 2;
constexpr unsigned int WINDOW_WIDTH = 800 * SCALE;
constexpr unsigned int WINDOW_HEIGHT = 600 * SCALE;

class Application
{
public:
    void run();

private:
    GLFWwindow *window = nullptr;
    std::optional<InputSystem> input;
    std::optional<Shader> defaultShader;
    std::optional<Shader> lightSourceShader;

    unsigned int VBO = 0, cubeVAO = 0, lightVAO = 0;
    unsigned int diffuseMapTexture = 0;
    unsigned int specularMapTexture = 0;
    unsigned int emissionMapTexture = 0;
    unsigned int blackTexture = 0;

    bool boringWhiteMode = true;

    Camera cam;
    std::vector<glm::vec3> cubePositions {
        { 0.0f,  0.0f,  0.0f},
        { 2.0f,  5.0f, -15.0f},
        {-1.5f, -2.2f, -2.5f},
        {-3.8f, -2.0f, -12.3f},
        { 2.4f, -0.4f, -3.5f},
        {-1.7f,  3.0f, -7.5f},
        { 1.3f, -2.0f, -2.5f},
        { 1.5f,  2.0f, -2.5f},
        { 1.5f,  0.2f, -1.5f},
        {-1.3f,  1.0f, -1.5f},
    };
    std::vector<glm::vec3> pointLightPositions = {
        { 0.7f, 0.2f, 2.0f },
        { 2.3f, -3.3f, -4.0f },
        { -4.0f, 2.0f, -12.0f },
        { 0.0f, 0.0f, -3.0f }
    };

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    bool wireframeMode = false;

    int fbWidth = 0;
    int fbHeight = 0;

    void startup();
    void process();
    void cleanup();
    void processInput();

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    static void loadTexture(unsigned int &textureId, const std::string &path);
    static void setupVertexAttributePointers();
};
