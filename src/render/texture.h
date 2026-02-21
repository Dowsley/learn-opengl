#pragma once
#include <string>
#include <glad/glad.h>


class Texture
{
public:
    unsigned int id;
    std::string type;
    std::string path;

    static unsigned int load(const std::string &path, GLenum wrapMode = GL_REPEAT);
    static unsigned int black();
};
