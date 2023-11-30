#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;

    glm::vec4 Color;

    float useDiffuseTexture;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;

    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        setupMesh();
    }

    void Draw(GLuint pipelineProgramId)
    {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); 
            string number;
            string name = textures[i].type;
            if (name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            //else if (name == "texture_specular")
            //    number = std::to_string(specularNr++); // transfer unsigned int to string
            //else if (name == "texture_normal")
            //    number = std::to_string(normalNr++); // transfer unsigned int to string
            //else if (name == "texture_height") 
            //    number = std::to_string(heightNr++); // transfer unsigned int to string


            glUniform1i(glGetUniformLocation(pipelineProgramId, (name + number).c_str()), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glActiveTexture(GL_TEXTURE0);


    }

private:
    unsigned int VBO, EBO;
    void setupMesh()
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
#endif