#pragma once
#include <string>
#include <unordered_map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "mesh.h"
#include "shader.h"

class Model
{
public:
    explicit Model(const std::string &path, GLenum wrapMode = GL_REPEAT);
    void draw(const Shader &shader) const;

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::unordered_map<std::string, Texture> textureCache;
    GLenum wrapMode;

    void loadModel(const std::string &path);
    void processNode(const aiNode *node, const aiScene *scene);
    Mesh processMesh(const aiMesh *mesh, const aiScene *scene);
    std::vector<Texture> loadMaterialTextures(const aiMaterial *mat, aiTextureType type, const std::string &typeName);
};
