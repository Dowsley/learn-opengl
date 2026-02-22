#pragma once

#include <memory>
#include <optional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "render/camera.h"
#include "render/model.h"
#include "render/shader.h"
#include "systems/input_system.h"
#include "voxel/voxel_world.h"
#include "voxel/voxel_renderer.h"

constexpr unsigned int SCALE = 2;
constexpr unsigned int WINDOW_WIDTH = 800 * SCALE;
constexpr unsigned int WINDOW_HEIGHT = 600 * SCALE;

class Application
{
public:
    void run();

private:
    const glm::vec3 WHITE{1.0};
    const glm::vec3 BLACK{0.0};

    GLFWwindow *window = nullptr;
    std::optional<InputSystem> input;
    std::optional<Shader> defaultShader;
    std::optional<Shader> lightSourceShader;

    Camera cam;
    std::unique_ptr<VoxelWorld> voxelWorld;
    std::unique_ptr<VoxelRenderer> voxelRenderer;
    std::optional<Model> backpack;
    std::optional<Model> container;
    std::optional<Model> cube;
    std::optional<Model> grass;
    std::optional<Model> transparentWindow;
    glm::vec3 pointLightPos = { 0.7f, 0.2f, 2.0f };
    std::vector<glm::vec3> grassPositions = {
        { -1.5f, 0.0f, -0.48f },
        { 1.5f, 0.0f, 0.51f },
        { 0.0f, 0.0f, 0.7f },
        { -0.3f, 0.0f, -2.3f },
        { 0.5f, 0.0f, -0.6f },
    };

    bool boringWhiteMode = true;
    bool flashlightOn = false;
    bool wireframeMode = false;
    bool lightPlacementMode = false;

    float lightPlacementModeDist = 3.0f;
    const float lightPlacementOffsetSpeed = 10.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    int fbWidth = 0;
    int fbHeight = 0;

    void startup();
    void process();
    void cleanup();
    void processInput();

    static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
};
