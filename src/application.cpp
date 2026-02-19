#include <iostream>
#include <vector>

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
    glfwSetScrollCallback(window, fovCallback);

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
        // positions          // colors          // tex coords
        // Back face (z = -0.5)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,   0.0f, 0.0f,
         // Front face (z = 0.5)
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         // Left face (x = -0.5)
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,   1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         // Right face (x = 0.5)
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         // Bottom face (y = -0.5)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,   0.0f, 1.0f,
         // Top face (y = 0.5)
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,   1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,   0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f,
     };

    /*
     * 3.1 Texture Setup
     * */
    glGenTextures(1, &texture1);

    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("assets/textures/container.jpg", &width,
                                    &height, &nChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture." << std::endl;
    }
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load("assets/textures/awesomeface.png", &width, &height,
                     &nChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture." << std::endl;
    }
    stbi_image_free(data);

    /*
     * 3.2 buffer objects setup
     * */
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    int stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    /* 3.3 Shader setup */
    shader.emplace("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");
    shader->use();
    shader->setInt("texture1", 0);
    shader->setInt("texture2", 1);

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

    float MIX_RATIO_INCREMENT = 0.01f;
    /* Input Processing */
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
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, up)) * speed * deltaTime;
    }
    if (isKeyPressed(GLFW_KEY_D))
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, up)) * speed * deltaTime;
    }
    if (isKeyPressed(GLFW_KEY_W))
    {
        cameraPos += cameraFront * speed * deltaTime;
    }
    if (isKeyPressed(GLFW_KEY_S))
    {
        cameraPos -= cameraFront * speed * deltaTime;
    }
    if (isKeyPressed(GLFW_KEY_LEFT_SHIFT))
    {
        cameraPos.y -= speed * deltaTime;
    }
    if (isKeyPressed(GLFW_KEY_SPACE))
    {
        cameraPos.y += speed * deltaTime;
    }

    /* Drawing/Rendering */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<glm::vec3> cubePositions = {
        glm::vec3( 0.0f, 0.0f, 0.0f),
        glm::vec3( 2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3( 2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3( 1.3f, -2.0f, -2.5f),
        glm::vec3( 1.5f, 2.0f, -2.5f),
        glm::vec3( 1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f)
    };

    auto viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, up);
    auto projectionMatrix = glm::perspective(
        glm::radians(fov), (float)fbWidth/(float)fbHeight, 0.1f, 100.0f);

    for (unsigned int i = 0; i < cubePositions.size(); i++)
    {
        auto modelMatrix = glm::mat4(1.0);
        modelMatrix = glm::translate(modelMatrix, cubePositions[i]);
        float angle = i % 3 == 0 ? glm::radians((float)glfwGetTime() * 100.0f) : 20.0f * (float)i;
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(1.0f, 0.5f, 0.0f));

        shader->use();
        shader->setFloat("mixRatio", mixRatio);
        shader->setMat4("modelMatrix", modelMatrix, 1, GL_FALSE);
        shader->setMat4("viewMatrix", viewMatrix, 1, GL_FALSE);
        shader->setMat4("projectionMatrix", projectionMatrix, 1, GL_FALSE);

        if (wireframeMode) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Application::cleanup()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    shader.reset();
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

    xOffset *= SENSITIVITY;
    yOffset *= SENSITIVITY;

    app->yaw += xOffset;
    app->pitch = glm::clamp(app->pitch + yOffset, -89.0f, 89.0f);

    auto direction = glm::vec3(
        glm::cos(glm::radians(app->yaw)) * glm::cos(glm::radians(app->pitch)),
        glm::sin(glm::radians(app->pitch)),
        glm::sin(glm::radians(app->yaw)) * glm::cos(glm::radians(app->pitch))
    );
    app->cameraFront = glm::normalize(direction);
}

void Application::fovCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
    app->fov = glm::clamp(app->fov - (float)yOffset, 1.0f, 45.0f);
}
