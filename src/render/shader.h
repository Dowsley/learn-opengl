#pragma once

#include <string>

#include <glm/fwd.hpp>

class Shader
{
public:
    unsigned int ID;

    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, glm::vec3 value) const;
    void setMat4(const std::string& name, glm::mat4 value, int count, int transpose) const;

private:
    static std::string _readFromFile(const std::string& filename);
    static unsigned int _compileShader(const char* shaderSource, int shaderType);
    static void _linkShaderToProgram(unsigned int shader, unsigned int program,
                                     int shaderType);
};
