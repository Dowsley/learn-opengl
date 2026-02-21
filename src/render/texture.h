#pragma once
#include <string>


class Texture
{
public:
    unsigned int id;
    std::string type;
    std::string path;

    static unsigned int load(const std::string &path);
};
