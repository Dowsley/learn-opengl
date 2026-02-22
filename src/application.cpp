#include <iostream>

#include <glad/glad.h>
#include "application.h"

void Application::run()
{
    startup();
    while (!glfwWindowShouldClose(window))
    {
        process();
    }
    cleanup();
}

void Application::startup()
{
    /* 1. GLFW: Set up context */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL - Dowsley", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    input.emplace(window);
    input->createAction("move_forward", {GLFW_KEY_W});
    input->createAction("move_backward", {GLFW_KEY_S});
    input->createAction("move_left", {GLFW_KEY_A});
    input->createAction("move_right", {GLFW_KEY_D});
    input->createAction("move_up", {GLFW_KEY_SPACE});
    input->createAction("move_down", {GLFW_KEY_LEFT_SHIFT});
    input->createAction("quit", {GLFW_KEY_ESCAPE});

    /* 2. GLAD: Initializing pointers */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    int nAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttributes);
    std::cout << "Maximum number of attributes: " << nAttributes << std::endl;

    /* 3. Voxel system */
    voxelWorld = std::make_unique<VoxelWorld>();
    voxelRenderer = std::make_unique<VoxelRenderer>();

    cam.pos = glm::vec3(128.0f, 160.0f, -40.0f);
    cam.pitch = -25.0f;
    cam.yaw = 90.0f;
    cam.speed = 26.0f;
    cam.processMouseMovement(0.0f, 0.0f); // update front vector

    /* 4. Prepare for main loop */
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
}

void Application::process()
{
    auto currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glfwPollEvents();
    processInput();

    /* Drawing/Rendering */
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    voxelRenderer->render(cam, fbWidth, fbHeight, *voxelWorld);

    glfwSwapBuffers(window);
}

void Application::cleanup()
{
    voxelRenderer.reset();
    voxelWorld.reset();
    glfwTerminate();
}

void Application::processInput()
{
    input->update();

    if (input->isActionJustPressed("quit"))
        glfwSetWindowShouldClose(window, true);

    if (input->isActionPressed("move_left"))
        cam.processKeyboard(LEFT, deltaTime);
    if (input->isActionPressed("move_right"))
        cam.processKeyboard(RIGHT, deltaTime);
    if (input->isActionPressed("move_forward"))
        cam.processKeyboard(FORWARD, deltaTime);
    if (input->isActionPressed("move_backward"))
        cam.processKeyboard(BACKWARD, deltaTime);
    if (input->isActionPressed("move_down"))
        cam.processKeyboard(DOWN, deltaTime);
    if (input->isActionPressed("move_up"))
        cam.processKeyboard(UP, deltaTime);

    auto mouseDelta = input->getMouseDelta();
    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        cam.processMouseMovement(mouseDelta.x, -mouseDelta.y);

    auto scrollDelta = input->getScrollDelta();
    if (scrollDelta.y != 0.0f)
        cam.processScroll(scrollDelta.y);
}

void Application::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    app->fbWidth = width;
    app->fbHeight = height;
    glViewport(0, 0, width, height);
}
