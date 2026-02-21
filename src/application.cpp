#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

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
    loadTexture(diffuseMapTexture, "assets/textures/container2.png");
    loadTexture(specularMapTexture, "assets/textures/container2_specular.png");
    loadTexture(emissionMapTexture, "assets/textures/matrix.jpg");

    // Cube (36 vertices: 6 faces x 2 triangles x 3 vertices)
    // Color per corner: (r,g,b) = (x+0.5, y+0.5, z+0.5)
    float vertices[] = {
        // positions          // normals           // texcoords
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
    };

    /*
     * 3.1 buffer objects setup
     * */
    /** 3.2.1: cubeVAO **/
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    setupVertexAttributePointers();

    /** 3.2.2: lightVAO **/
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    setupVertexAttributePointers();

    /* 3.2 Shader setup */
    lightSourceShader.emplace("shaders/vertexShaderDefault.glsl", "shaders/fragmentShaderLightSource.glsl");
    defaultShader.emplace("shaders/vertexShaderDefault.glsl", "shaders/fragmentShaderPhong.glsl");
    defaultShader->use();
    defaultShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));


    /* 4. Prepare for main loop */
    glEnable(GL_DEPTH_TEST);
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

    // 1. Cube
    auto viewMatrix = cam.getViewMatrix();
    auto projectionMatrix = glm::perspective(
        glm::radians(cam.fov), (float)fbWidth/(float)fbHeight, 0.1f, 100.0f);

    defaultShader->use();
    defaultShader->setMat4("viewMatrix", viewMatrix, 1, GL_FALSE);
    defaultShader->setMat4("projectionMatrix", projectionMatrix, 1, GL_FALSE);
    defaultShader->setVec3("viewPos", cam.pos);
    defaultShader->setInt("material.diffuseMap", 0);
    defaultShader->setInt("material.specularMap", 1);
    defaultShader->setFloat("material.shininess", 32.0f);

    glm::vec3 white(1.0);
    defaultShader->setVec3("dirLight.direction", { 0.0f, -1.0f, 0.0f });
    defaultShader->setVec3("dirLight.ambientColor", white * glm::vec3(.1f));
    defaultShader->setVec3("dirLight.diffuseColor", white * glm::vec3(1.0f));
    defaultShader->setVec3("dirLight.specularColor", white * glm::vec3(1.0f));

    glm::vec3 pointLightColor = boringWhiteMode
        ? white
        : glm::vec3{ sin(currentFrame * 2.0f), sin(currentFrame * 0.7f), sin(currentFrame * 1.3f) };
    for (size_t i = 0; i < pointLightPositions.size(); i++)
    {
        auto pointPos = pointLightPositions[i];

        std::string prefix = "pointLights[" + std::to_string(i) + "].";
        defaultShader->setVec3(prefix + "position", pointPos);
        defaultShader->setVec3(prefix + "ambientColor", pointLightColor * glm::vec3(0.02f));
        defaultShader->setVec3(prefix + "diffuseColor", pointLightColor * glm::vec3(0.6f));
        defaultShader->setVec3(prefix + "specularColor", glm::vec3(1.0f, 1.0f, 1.0f));
        defaultShader->setFloat(prefix + "constantAttTerm", 1.0f);
        defaultShader->setFloat(prefix + "linearAttTerm", 0.09f);
        defaultShader->setFloat(prefix + "quadraticAttTerm", 0.032f);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMapTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMapTexture);
    glBindVertexArray(cubeVAO);

    for (size_t i = 0; i < cubePositions.size(); i++)
    {
        auto modelMatrix = glm::translate(glm::mat4(1.0f), cubePositions[i]);
        float angle = 20.0f * (float)i;
        modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        defaultShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 2. Light sources
    lightSourceShader->use();
    lightSourceShader->setMat4("viewMatrix", viewMatrix, 1, GL_FALSE);
    lightSourceShader->setMat4("projectionMatrix", projectionMatrix, 1, GL_FALSE);

    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(lightVAO);
    for (auto pointPos : pointLightPositions)
    {
        auto modelMatrix = glm::translate(glm::mat4(1.0f), pointPos);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));

        lightSourceShader->setVec3("color", pointLightColor);
        lightSourceShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::cleanup()
{
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);
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
    if (input->isActionJustPressed("toggle_light_mode"))
        boringWhiteMode = !boringWhiteMode;
}

void Application::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    app->fbWidth = width;
    app->fbHeight = height;
    glViewport(0, 0, width, height);
}


void Application::loadTexture(unsigned int &textureId, const std::string &path)
{
    glGenTextures(1, &textureId);

    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    auto format = path.ends_with(".png") ? GL_RGBA : GL_RGB;
    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture." << std::endl;
    }
    stbi_image_free(data);
}

void Application::setupVertexAttributePointers()
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
}


