#pragma once
#include <vector>

#include "shader.h"
#include "texture.h"
#include "vertex.h"

class Mesh
{
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
    void draw(const Shader &shader) const;

private:
    unsigned int VAO, VBO, EBO;
    void setupMesh();
};
