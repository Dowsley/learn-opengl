//
// Created by fell on 5/27/24.
//

#ifndef LEARN_OPENGL_SHADER_H
#define LEARN_OPENGL_SHADER_H

#include <string>

class Shader
{
public:
    unsigned int ID;

    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    void use() const;

    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

private:
    static std::string _readFromFile(const std::string& filename);
    static unsigned int _compileShader(const char* shaderSource, int shaderType);
    static void _linkShaderToProgram(unsigned int shader, unsigned int program,
                                     int shaderType);
};

#endif // LEARN_OPENGL_SHADER_H
