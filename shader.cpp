//
// Created by fell on 5/27/24.
//

#include <fstream>
#include <string>

#include <glad/glad.h>
#include <iostream>
#include "shader.h"

Shader::Shader(const std::string &vertexPath, const std::string &fragmentPath) {
    const std::string vertexShaderSource = _readFromFile(vertexPath);
    const std::string fragmentShaderSource = _readFromFile(fragmentPath);

    unsigned int vertexShader = _compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = _compileShader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);

    ID = glCreateProgram();
    _linkShaderToProgram(vertexShader, ID, GL_FRAGMENT_SHADER);
    _linkShaderToProgram(fragmentShader, ID, GL_FRAGMENT_SHADER);

    glLinkProgram(ID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int) value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

const std::string Shader::_readFromFile(const std::string &filename) const
{
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open()) {
        while (std::getline(myFile, line)) {
            result += line + '\n';
        }
        myFile.close();
    }
    return result;
}

unsigned int Shader::_compileShader(const char *shaderSource, int shaderType) const
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

void Shader::_linkShaderToProgram(unsigned int shader, unsigned int program, int shaderType) const
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
