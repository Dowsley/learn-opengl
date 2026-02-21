#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
    input->createAction("toggle_wireframe", {GLFW_KEY_LEFT_ALT});
    input->createAction("toggle_light_mode", {GLFW_KEY_Q});
    input->createAction("toggle_light_placement", {GLFW_KEY_E});
    input->createAction("toggle_flashlight", {GLFW_KEY_F});

    /* 2. GLAD: Initializing pointers */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    int nAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttributes);
    std::cout << "Maximum number of attributes: " << nAttributes << std::endl;

    /* 3. OpenGL: Initializing shaders and objects */
    backpack.emplace("assets/models/backpack/backpack.obj");
    container.emplace("assets/models/container/container.obj");
    cube.emplace("assets/models/cube/cube.obj");
    grass.emplace("assets/models/grass/grass.obj", GL_CLAMP_TO_EDGE);
    transparentWindow.emplace("assets/models/window/window.obj", GL_CLAMP_TO_EDGE);

    /* 3.2 Shader setup */
    lightSourceShader.emplace("shaders/vertexShaderDefault.glsl", "shaders/fragmentShaderLightSource.glsl");
    defaultShader.emplace("shaders/vertexShaderDefault.glsl", "shaders/fragmentShaderPhong.glsl");

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

    processInput();

    /* Drawing/Rendering */
    // glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // 1. cube
    auto viewMatrix = cam.getViewMatrix();
    auto projectionMatrix = glm::perspective(
        glm::radians(cam.fov), (float)fbWidth/(float)fbHeight, 0.1f, 100.0f);

    defaultShader->use();
    defaultShader->setMat4("viewMatrix", viewMatrix, 1, GL_FALSE);
    defaultShader->setMat4("projectionMatrix", projectionMatrix, 1, GL_FALSE);
    defaultShader->setVec3("viewPos", cam.pos);
    defaultShader->setFloat("shininess", 32.0f);

    defaultShader->setVec3("dirLight.direction", { 0.0f, -1.0f, 0.0f });
    defaultShader->setVec3("dirLight.ambientColor", WHITE * glm::vec3(.1f));
    defaultShader->setVec3("dirLight.diffuseColor", WHITE * glm::vec3(1.0f));
    defaultShader->setVec3("dirLight.specularColor", WHITE * glm::vec3(1.0f));

    glm::vec3 pointLightColor = boringWhiteMode
        ? WHITE
        : glm::vec3{ sin(currentFrame * 2.0f), sin(currentFrame * 0.7f), sin(currentFrame * 1.3f) };
    defaultShader->setVec3("pointLights[0].position", pointLightPos);
    defaultShader->setVec3("pointLights[0].ambientColor", pointLightColor * glm::vec3(0.02f));
    defaultShader->setVec3("pointLights[0].diffuseColor", pointLightColor * glm::vec3(0.6f));
    defaultShader->setVec3("pointLights[0].specularColor", glm::vec3(1.0f, 1.0f, 1.0f));
    defaultShader->setFloat("pointLights[0].constantAttTerm", 1.0f);
    defaultShader->setFloat("pointLights[0].linearAttTerm", 0.09f);
    defaultShader->setFloat("pointLights[0].quadraticAttTerm", 0.032f);

    // spotlight (flashlight attached to camera)
    defaultShader->setVec3("spotLight.position", cam.pos);
    defaultShader->setVec3("spotLight.direction", cam.front);
    defaultShader->setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    defaultShader->setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
    defaultShader->setVec3("spotLight.ambientColor", BLACK);
    defaultShader->setVec3("spotLight.diffuseColor", flashlightOn ? WHITE : BLACK);
    defaultShader->setVec3("spotLight.specularColor", flashlightOn ? WHITE : BLACK);
    defaultShader->setFloat("spotLight.constantAttTerm", 1.0f);
    defaultShader->setFloat("spotLight.linearAttTerm", 0.027f);
    defaultShader->setFloat("spotLight.quadraticAttTerm", 0.0028f);

    auto modelMatrix = glm::mat4(1.0f);
    defaultShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
    backpack->draw(*defaultShader);

    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
    defaultShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
    container->draw(*defaultShader);

    for (unsigned int i = 0; i < grassPositions.size(); i++)
    {
        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-5.0f, 0.0f, -1.0f));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        defaultShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
        grass->draw(*defaultShader);
    }

    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 2.0f));
    defaultShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
    transparentWindow->draw(*defaultShader);

    // 3. light sources
    lightSourceShader->use();
    lightSourceShader->setMat4("viewMatrix", viewMatrix, 1, GL_FALSE);
    lightSourceShader->setMat4("projectionMatrix", projectionMatrix, 1, GL_FALSE);

    modelMatrix = glm::translate(glm::mat4(1.0f), pointLightPos);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));
    lightSourceShader->setVec3("color", pointLightColor);
    lightSourceShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
    cube->draw(*lightSourceShader);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::cleanup()
{
    defaultShader.reset();
    lightSourceShader.reset();
    glfwTerminate();
}

void Application::processInput()
{
    input->update();

    if (input->isActionJustPressed("quit"))
        glfwSetWindowShouldClose(window, true);

    if (input->isActionJustPressed("toggle_wireframe"))
        wireframeMode = !wireframeMode;

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

    // light config
    if (input->isActionJustPressed("toggle_flashlight"))
        flashlightOn = !flashlightOn;
    if (input->isActionJustPressed("toggle_light_mode"))
        boringWhiteMode = !boringWhiteMode;

    if (input->isActionJustPressed("toggle_light_placement"))
        lightPlacementMode = !lightPlacementMode;

    if (lightPlacementMode) {
        pointLightPos = cam.pos + cam.front*lightPlacementModeDist;
        if (input->isMouseButtonPressed(GLFW_MOUSE_BUTTON_1))
            lightPlacementModeDist += lightPlacementOffsetSpeed*deltaTime;
        if (input->isMouseButtonPressed(GLFW_MOUSE_BUTTON_2))
            lightPlacementModeDist -= lightPlacementOffsetSpeed*deltaTime;
    }
}

void Application::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    app->fbWidth = width;
    app->fbHeight = height;
    glViewport(0, 0, width, height);
}
