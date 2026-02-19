#pragma once

#include <optional>
#include <GLFW/glfw3.h>

#include "render/camera.h"
#include "render/shader.h"

constexpr unsigned int SCALE = 2;
constexpr unsigned int WINDOW_WIDTH = 800 * SCALE;
constexpr unsigned int WINDOW_HEIGHT = 600 * SCALE;

class Application
{
public:
    void run();

private:
    GLFWwindow *window = nullptr;
    std::optional<Shader> defaultShader;
    std::optional<Shader> lightSourceShader;

    unsigned int VBO = 0, cubeVAO = 0, lightVAO = 0;

    Camera cam;
    glm::vec3 cubePosition{ 0.0f, 0.0f, 0.0f };
    glm::vec3 lightSourcePosition{ 1.2f, 1.0f, 2.0f };

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float lastX = WINDOW_WIDTH / 2.0f;
    float lastY = WINDOW_HEIGHT / 2.0f;
    bool firstMouse = true;

    float mixRatio = 0.2f;
    bool wireframeMode = false;

    int fbWidth = 0;
    int fbHeight = 0;

    void startup();
    void process();
    void cleanup();
    bool isKeyPressed(int key) const;

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
    static void mouseCallback(GLFWwindow *window, double xPos, double yPos);
    static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);
};
