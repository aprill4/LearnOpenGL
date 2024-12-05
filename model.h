#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
// glad must be included before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_map>
#include <filesystem>
namespace fs = std::filesystem;

#include "mesh.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::cerr, std::endl, std::unordered_map;

static std::string getPath(const std::string& path) {
    fs::path fsPath(path);
    return fsPath.make_preferred().string();
}

unsigned loadTextureFromFile(const std::string &imagePath);

class Model {
public:
    Model(const string &path) {
        loadModel(path);
    }

    void Draw(Shader &shader) {
        for (unsigned i = 0; i < meshes.size(); ++i) {
            meshes[i].Draw(shader);
        }
    }

private:
    vector<Mesh> meshes;
    string directory;
    unordered_map<string, Texture> textures_loaded;

    void loadModel(const string &path);
    void processNode(const aiNode *node, const aiScene *scene);
    Mesh processMesh(const aiMesh *mesh, const aiScene *scene);
    vector<Texture> loadMaterialTextures(const aiMaterial *mat, const aiTextureType type, const string &typeName);
};

void Model::loadModel(const string &path) {
    Assimp::Importer import;
    const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cerr << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
    }

    directory = path.substr(0, path.find_last_of("/\\"));
    cerr << directory << endl;

    processNode(scene->mRootNode, scene);
}

void Model::processNode(const aiNode *node, const aiScene *scene) {
    // iterate throuth all the nodes
    for (unsigned i = 0; i < node->mNumMeshes; ++i) {
        meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
    }

    for (unsigned i = 0; i < node->mNumChildren; ++i) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(const aiMesh *mesh, const aiScene *scene) {
    // Vertex
    vector<Vertex> vertices;
    for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        } else {
            vertex.TexCoords = glm::vec2(0.f, 0.f);
        }
        vertices.push_back(vertex);
    }
    // Indices
    vector<unsigned> indices;
    for (unsigned i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }
    // Textures
    vector<Texture> textures;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }

    return Mesh(vertices, textures, indices);
}

vector<Texture> Model::loadMaterialTextures(const aiMaterial *mat, const aiTextureType type, const string &typeName) {
    vector<Texture> textures;
    for (unsigned i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        if (textures_loaded.find(str.data) != textures_loaded.end()) {
            textures.push_back(textures_loaded[str.data]);
            skip = true;
            break;
        }

        if (!skip) {
            Texture texture;
            texture.ID = loadTextureFromFile(getPath(directory + "/" + str.data));
            texture.Type = typeName;
            texture.Path = str.data;
            textures.push_back(texture);
            textures_loaded[str.data] = texture;
        }
    }
    return textures;
}

unsigned loadTextureFromFile(const std::string &imagePath) {
  unsigned textureID;
  glGenTextures(1, &textureID);

  int width, height, nrChannels;
  unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);

  if (data) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    // copy data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

  } else {
    cerr << "ERROR: Failed to load texture: " << imagePath << endl;
  }

  stbi_image_free(data);

  return textureID;
}

#endif
