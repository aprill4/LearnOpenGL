#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
// glad must be included before GLFW
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

#include "shader.h"

using std::string, std::vector;

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Texture {
    unsigned ID;
    string Type;
    string Path;
};

class Mesh {
public:
    vector<Vertex> vertices;
    vector<Texture> textures;
    vector<unsigned> indices;

    Mesh(const vector<Vertex> &vertices, const vector<Texture> &textures, const vector<unsigned> &indices) : vertices(vertices), textures(textures), indices(indices) {
        setUp();
    }

    void Draw(Shader &shader) {
        unsigned nDiffuse = 1;
        unsigned nSpecular = 1;

        for (int i = 0; i < textures.size(); ++i) {
            glActiveTexture(GL_TEXTURE0 + i);

            string number; 
            string name = textures[i].Type;
            if (name == "texture_diffuse") {
                number = std::to_string(nDiffuse++);
            } else if (name == "texture_specular") {
                number = std::to_string(nSpecular++);
            }

            shader.setInt((name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].ID);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

private:
    unsigned VAO, VBO, EBO;
    void setUp() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), &indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);

        // glBindVertexArray(0);
        // glBindBuffer(GL_ARRAY_BUFFER, 0);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};

#endif

