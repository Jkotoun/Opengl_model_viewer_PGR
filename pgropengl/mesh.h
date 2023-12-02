#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

//inspired by learnopengl tutorial - basic concept of loading model using assimp
//https://learnopengl.com/Model-Loading/Assimp

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec4 Color;
    float useDiffuseTexture;
};

struct Texture {
    unsigned int id;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    bool hasTexture;
    Texture      texture;
    bool isTransparent;
    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Texture texture, bool hasTexture, bool isTransparent)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->texture = texture;
        this->isTransparent = isTransparent;
        this->hasTexture = hasTexture;

        setupOpenGLBuffers();
    }

    void Draw(GLuint pipelineProgramId)
    {
        if (this->hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glUniform1i(glGetUniformLocation(pipelineProgramId, "texture_diffuse0"), 0);
            glBindTexture(GL_TEXTURE_2D, texture.id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:
    unsigned int VBO, EBO;
    void setupOpenGLBuffers()
    {

        glCreateVertexArrays(1, &VAO);

        glCreateBuffers(1, &EBO);
        glNamedBufferData(EBO, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(VAO, EBO);

        glCreateBuffers(1, &VBO);
        glNamedBufferData(VBO, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);


        //position
        glVertexArrayAttribBinding(VAO, 0, 0);
        glEnableVertexArrayAttrib(VAO, 0);
        glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayVertexBuffer(VAO, 0, VBO,0, sizeof(Vertex));

        //normals
        glVertexArrayAttribBinding(VAO, 1, 0);
        glEnableVertexArrayAttrib(VAO, 1);
        glVertexArrayAttribFormat(VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, Normal));
        glVertexArrayVertexBuffer(VAO, 1, VBO, 0, sizeof(Vertex));

        //textures
        glVertexArrayAttribBinding(VAO, 2, 0);
        glEnableVertexArrayAttrib(VAO, 2);
        glVertexArrayAttribFormat(VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, TexCoords));
        glVertexArrayVertexBuffer(VAO, 2, VBO, 0, sizeof(Vertex));


        //color
        glVertexArrayAttribBinding(VAO, 3, 0);
        glEnableVertexArrayAttrib(VAO, 3);
        glVertexArrayAttribFormat(VAO, 3, 4, GL_FLOAT, GL_FALSE, offsetof(Vertex, Color));
        glVertexArrayVertexBuffer(VAO, 3, VBO, 0, sizeof(Vertex));
            
        //useDiffuseTexture
        glVertexArrayAttribBinding(VAO, 4, 0);
        glEnableVertexArrayAttrib(VAO, 4);
        glVertexArrayAttribFormat(VAO, 4, 1, GL_FLOAT, GL_FALSE, offsetof(Vertex, useDiffuseTexture));
        glVertexArrayVertexBuffer(VAO, 4, VBO, 0, sizeof(Vertex));

        glBindVertexArray(0);
    }
};
