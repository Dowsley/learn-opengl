#pragma once

#include <memory>
#include <optional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "render/camera.h"
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
    GLFWwindow *window = nullptr;
    std::optional<InputSystem> input;

    Camera cam;
    std::unique_ptr<VoxelWorld> voxelWorld;
    std::unique_ptr<VoxelRenderer> voxelRenderer;

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
