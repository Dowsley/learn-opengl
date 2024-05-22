#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
bool isKeyPressed(GLFWwindow *window, int key);
unsigned int createShader(const char *shaderSource, int shaderType);
void linkShaderToProgram(unsigned int shader, unsigned int program, int shaderType);
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

    /* 3. OpenGL: Initializing shaders and objects */
    float vertices1[] = {
        -0.5f, 0.5f, 0.0f,
        -1.0f, -0.5f, 0.0f,
        0.0f, -0.5f, 0.0f,
    };
    float vertices2[] = {
        0.0f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        1.0f, -0.5f, 0.0f
    };
    unsigned int VAO1 = createPrimitiveTriangleVAO(vertices1, sizeof(vertices1));
    unsigned int VAO2 = createPrimitiveTriangleVAO(vertices2, sizeof(vertices2));

    const char *vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
        "}\0";
    const char *fragmentShaderSource1 =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";
    const char *fragmentShaderSource2 =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
        "}\0";
    unsigned int shaderProgram1 = glCreateProgram();
    unsigned int shaderProgram2 = glCreateProgram();

    unsigned int vertexShader = createShader(vertexShaderSource, GL_VERTEX_SHADER);
    linkShaderToProgram(vertexShader, shaderProgram1, GL_FRAGMENT_SHADER);
    linkShaderToProgram(vertexShader, shaderProgram2, GL_FRAGMENT_SHADER);

    unsigned int fragmentShader1 = createShader(fragmentShaderSource1, GL_FRAGMENT_SHADER);
    linkShaderToProgram(fragmentShader1, shaderProgram1, GL_FRAGMENT_SHADER);
    unsigned int fragmentShader2 = createShader(fragmentShaderSource2, GL_FRAGMENT_SHADER);
    linkShaderToProgram(fragmentShader2, shaderProgram2, GL_FRAGMENT_SHADER);
    glLinkProgram(shaderProgram1);
    glLinkProgram(shaderProgram2);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader1);
    glDeleteShader(fragmentShader2);

    std::cout << "Starting window and main loop..." << std::endl;
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram1);
        glBindVertexArray(VAO1);
        glDrawArrays(GL_TRIANGLES, 0, 5);
        glBindVertexArray(0);

        glUseProgram(shaderProgram2);
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 5);
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

unsigned int createShader(const char *shaderSource, int shaderType)
{
    unsigned int shader;
    shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    const unsigned int INFO_LOG_SIZE = 512;
    int success;
    char infoLog[INFO_LOG_SIZE];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, INFO_LOG_SIZE, nullptr, infoLog);
        std::cout << "ERROR::SHADER::" << std::to_string(shaderType) << "::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    return shader;
}

void linkShaderToProgram(unsigned int shader, unsigned int program, int shaderType)
{
    const unsigned int INFO_LOG_SIZE = 512;
    int success;
    char infoLog[INFO_LOG_SIZE];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    glAttachShader(program, shader);
    if (!success) {
        glGetShaderInfoLog(shader, INFO_LOG_SIZE, nullptr, infoLog);
        std::cout << "ERROR::SHADER::" << std::to_string(shaderType) << "::ATTACH_FAILED\n" << infoLog << std::endl;
    }
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);

    return VAO;
}
