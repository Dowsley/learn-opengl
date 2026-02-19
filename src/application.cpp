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
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

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
    // Cube (36 vertices: 6 faces x 2 triangles x 3 vertices)
    // Color per corner: (r,g,b) = (x+0.5, y+0.5, z+0.5)
    float vertices[] = {
        // positions          // normals
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        // Left face
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        // Right face
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
        // Bottom face
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
        // Top face
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
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

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /** 3.2.2: lightVAO **/
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    /* 3.2 Shader setup */
    lightSourceShader.emplace("shaders/vertexShaderDefault.glsl", "shaders/fragmentShaderLightSource.glsl");
    defaultShader.emplace("shaders/vertexShaderDefault.glsl", "shaders/fragmentShaderPhong.glsl");
    defaultShader->use();
    defaultShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    defaultShader->setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));


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

    /* Input Processing */
    float MIX_RATIO_INCREMENT = 0.01f;
    if (isKeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (isKeyPressed(GLFW_KEY_UP))
    {
        mixRatio += MIX_RATIO_INCREMENT;
        if (mixRatio > 1.0)
        {
            mixRatio = 1.0;
        }
    }
    if (isKeyPressed(GLFW_KEY_DOWN))
    {
        mixRatio -= MIX_RATIO_INCREMENT;
        if (mixRatio < 0.0)
        {
            mixRatio = 0.0;
        }
    }
    if (isKeyPressed(GLFW_KEY_LEFT_ALT))
    {
        wireframeMode = !wireframeMode;
    }
    if (isKeyPressed(GLFW_KEY_A))
        cam.processKeyboard(LEFT, deltaTime);
    if (isKeyPressed(GLFW_KEY_D))
        cam.processKeyboard(RIGHT, deltaTime);
    if (isKeyPressed(GLFW_KEY_W))
        cam.processKeyboard(FORWARD, deltaTime);
    if (isKeyPressed(GLFW_KEY_S))
        cam.processKeyboard(BACKWARD, deltaTime);
    if (isKeyPressed(GLFW_KEY_LEFT_SHIFT))
        cam.processKeyboard(DOWN, deltaTime);
    if (isKeyPressed(GLFW_KEY_SPACE))
        cam.processKeyboard(UP, deltaTime);

    /* Drawing/Rendering */
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
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

    auto modelMatrix = glm::translate(glm::mat4(1.0), cubePosition);
    // modelMatrix = glm::rotate(modelMatrix, 0.0f, glm::vec3(1.0f, 0.5f, 0.0f));

    defaultShader->use();
    defaultShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
    defaultShader->setMat4("viewMatrix", viewMatrix, 1, GL_FALSE);
    defaultShader->setMat4("projectionMatrix", projectionMatrix, 1, GL_FALSE);
    defaultShader->setVec3("lightPos", lightSourcePosition);
    defaultShader->setVec3("viewPos", cam.pos);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // 2. Light source
    modelMatrix = glm::translate(glm::mat4(1.0), lightSourcePosition);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f));

    lightSourceShader->use();
    lightSourceShader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
    lightSourceShader->setMat4("viewMatrix", viewMatrix, 1, GL_FALSE);
    lightSourceShader->setMat4("projectionMatrix", projectionMatrix, 1, GL_FALSE);

    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

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

bool Application::isKeyPressed(int key) const
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void Application::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Application::mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));

    if (app->firstMouse)
    {
        app->lastX = xPos;
        app->lastY = yPos;
        app->firstMouse = false;
    }

    float xOffset = xPos - app->lastX;
    float yOffset = app->lastY - yPos;
    app->lastX = xPos;
    app->lastY = yPos;

    app->cam.processMouseMovement(xOffset, yOffset);
}

void Application::scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    app->cam.processScroll((float)yOffset);
}
