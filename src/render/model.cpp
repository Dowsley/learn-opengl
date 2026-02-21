#include "model.h"

#include <iostream>
#include <assimp/postprocess.h>
#include <stb_image.h>

Model::Model(const std::string &path, GLenum wrapMode)
    : wrapMode(wrapMode)
{
    loadModel(path);
}

void Model::draw(const Shader &shader) const
{
    for (Mesh mesh : meshes)
    {
        mesh.draw(shader);
    }
}

void Model::loadModel(const std::string &path)
{
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR:ASSIMP::" << import.GetErrorString() << std::endl;
        return;
    }

    directory = path.substr(0, path.find_last_of('/'));
    processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode *node, const aiScene *scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(const aiMesh *mesh, const aiScene *scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        auto v = mesh->mVertices[i];
        auto n = mesh->mNormals[i];
        auto texCoords = mesh->mTextureCoords[0]; // there can be up to 8 per vertex, but I only care about one

        Vertex vertex{
            { v.x, v.y, v.z },
            { n.x, n.y, n.z },
            texCoords ? glm::vec2{ texCoords[i].x, texCoords[i].y } : glm::vec2{0.0f, 0.0f }
        };
        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(const aiMaterial *mat, aiTextureType type, const std::string &typeName)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string filepath = directory + '/' + str.C_Str();

        auto it = textureCache.find(filepath);
        if (it != textureCache.end())
        {
            textures.push_back(it->second);
        }
        else
        {
            Texture tex { Texture::load(filepath, wrapMode), typeName, filepath };
            textureCache[filepath] = tex;
            textures.push_back(tex);
        }
    }
    return textures;
}
