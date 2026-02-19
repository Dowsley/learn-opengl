//
// Created by fell on 5/27/24.
//

#include <fstream>
#include <string>

#include "shader.h"
#include <glad/glad.h>
#include <iostream>

#include "fwd.hpp"
#include "gtc/type_ptr.hpp"

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath)
{
    const std::string vertexShaderSource = _readFromFile(vertexPath);
    const std::string fragmentShaderSource = _readFromFile(fragmentPath);

    unsigned int vertexShader =
        _compileShader(vertexShaderSource.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader =
        _compileShader(fragmentShaderSource.c_str(), GL_FRAGMENT_SHADER);

    ID = glCreateProgram();
    _linkShaderToProgram(vertexShader, ID, GL_FRAGMENT_SHADER);
    _linkShaderToProgram(fragmentShader, ID, GL_FRAGMENT_SHADER);

    glLinkProgram(ID);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, glm::mat4 value, int count, int transpose) const
{
    unsigned int modelLoc = glGetUniformLocation(ID, name.c_str());
    glUniformMatrix4fv((int)modelLoc, count, transpose, glm::value_ptr(value));
}

std::string Shader::_readFromFile(const std::string& filename)
{
    std::string result;

    std::ifstream myFile(filename.c_str());

    if (myFile.is_open())
    {
        std::string line;
        while (std::getline(myFile, line))
        {
            result += line + '\n';
        }
        myFile.close();
    }
    return result;
}

unsigned int Shader::_compileShader(const char* shaderSource,
                                    int shaderType)
{
    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    constexpr unsigned int INFO_LOG_SIZE = 512;
    int success;
    char infoLog[INFO_LOG_SIZE];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, INFO_LOG_SIZE, nullptr, infoLog);
        std::cout << "ERROR::SHADER::" << std::to_string(shaderType)
            << "::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }
    return shader;
}

void Shader::_linkShaderToProgram(unsigned int shader, unsigned int program, int shaderType)
{
    constexpr unsigned int INFO_LOG_SIZE = 512;
    int success;
    char infoLog[INFO_LOG_SIZE];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    glAttachShader(program, shader);
    if (!success)
    {
        glGetShaderInfoLog(shader, INFO_LOG_SIZE, nullptr, infoLog);
        std::cout << "ERROR::SHADER::" << std::to_string(shaderType)
            << "::ATTACH_FAILED\n"
            << infoLog << std::endl;
    }
}
