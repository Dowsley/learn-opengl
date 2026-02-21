#pragma once

#include <optional>
#include <GLFW/glfw3.h>

#include "render/camera.h"
#include "render/model.h"
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

    unsigned int VBO = 0, lightVAO = 0;
    std::optional<Model> backpack;

    const glm::vec3 WHITE{1.0};
    const glm::vec3 BLACK{0.0};
    bool boringWhiteMode = true;
    bool flashlightOn = false;

    Camera cam;
    glm::vec3 pointLightPos = { 0.7f, 0.2f, 2.0f };
    bool lightPlacementMode = false;
    float lightPlacementModeDist = 3.0f;
    float lightPlacementOffsetSpeed = 10.0f;

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
    static void setupVertexAttributePointers();
};
