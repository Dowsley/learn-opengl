#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iterator>
#include <cmath>
#include "shader.h"

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
bool isKeyPressed(GLFWwindow *window, int key);
unsigned int createPrimitiveTriangleVAO(float vertices[], int size);

int main()
{
    /* 1. GLFW: Set up context */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(
            WINDOW_WIDTH, WINDOW_HEIGHT,
            "LearnOpenGL - fellsand", nullptr, nullptr);
    if (window == nullptr) {
    	std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    /* 2. GLAD: Initializing pointers */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	    std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int nAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttributes);
    std::cout << "Maximum number of attributes: " << nAttributes << std::endl;


    /* 3. OpenGL: Initializing shaders and objects */
    float vertices[] = {
        0.0, 0.5, 0.0,      1.0, 0.0, 0.0,
        -0.5, -0.5, 0.0,        0.0, 1.0, 0.0,
        0.5,-0.5, 0.0,      0.0, 0.0, 1.0
    };
    unsigned int VAO = createPrimitiveTriangleVAO(vertices, sizeof(vertices));

    Shader ourShader("shaders/vertexShader.glsl", "shaders/fragmentShader.glsl");

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.use();

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

/* Will serve as a callback for GLFW whenever the window is resized */
void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (isKeyPressed(window, GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }
}

bool isKeyPressed(GLFWwindow *window, int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

unsigned int createPrimitiveTriangleVAO(float vertices[], int size)
{
    /* We create and bind a VAO first, so it registers the VBO and Vertex Attribute calls. */
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /* Copy the vertex data to the actual VBO and bind it */
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Static draw because the triangle won't change for now.
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

    /* Set the vertex attribute pointers */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    return VAO;
}


