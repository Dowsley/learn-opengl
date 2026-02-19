#ifndef APPLICATION_H
#define APPLICATION_H

#include <optional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include "render/shader.h"

constexpr unsigned int SCALE = 2;
constexpr unsigned int WINDOW_WIDTH = 800 * SCALE;
constexpr unsigned int WINDOW_HEIGHT = 600 * SCALE;
constexpr float SENSITIVITY = 0.1f;

class Application
{
public:
    void run();

private:
    GLFWwindow *window = nullptr;
    std::optional<Shader> shader;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int texture1 = 0;
    unsigned int texture2 = 0;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float lastX = WINDOW_WIDTH / 2.0f;
    float lastY = WINDOW_HEIGHT / 2.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;
    float speed = 6.5f;
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
    static void fovCallback(GLFWwindow *window, double xOffset, double yOffset);
};

#endif // APPLICATION_H
