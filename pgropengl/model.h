#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

unsigned int TextureFromFile(const char* path, const std::string& directory);

class Model
{
public:
    std::vector<Mesh> opaqueMeshes;
    std::vector<Mesh> transparentMeshes;
    std::string directory;

    Model(std::string const& path) 
    {
        loadModel(path);
    }

    void Draw(GLuint pipelineProgramId) 
	{
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        for (unsigned int i = 0; i < opaqueMeshes.size(); i++) {
            opaqueMeshes[i].Draw(pipelineProgramId);
        }

        glDepthMask(GL_FALSE);

        for (unsigned int i = 0; i < transparentMeshes.size(); i++) {
            transparentMeshes[i].Draw(pipelineProgramId);
        }
        glDepthMask(GL_TRUE);
	}

private:
    void loadModel(std::string const& path)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
           std::cout << importer.GetErrorString() << std::endl;
            return;
        }

        // retrieve the directory path of the filepath - we assume that textures are in same directory
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            Mesh m = processMesh(scene->mMeshes[node->mMeshes[i]], scene);
            if (m.isTransparent)
			{
				transparentMeshes.push_back(m);
			}
			else
			{
				opaqueMeshes.push_back(m);
			}

        }
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        const auto& mat = scene->mMaterials[mesh->mMaterialIndex];

        glm::vec4 color = glm::vec4(1.f, 1.f, 1.f, 1.f);
        float useDiffuseTexture = 0.f;
        aiColor4D diffuse;
        float alpha;
        if (aiGetMaterialColor(mat, AI_MATKEY_COLOR_DIFFUSE, &diffuse) == AI_SUCCESS && aiGetMaterialFloat(mat, AI_MATKEY_OPACITY, &alpha) == AI_SUCCESS)
        {
            color= glm::vec4(diffuse.r, diffuse.g, diffuse.b, alpha);
        }

        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            useDiffuseTexture = 1.f;
        }
        else
        {
            useDiffuseTexture = 0.f;
        }

    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; 
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            //limitation for only one texturing coordinate set
            if (mesh->mTextureCoords[0]) 
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            vertex.Color = color;
            vertex.useDiffuseTexture = useDiffuseTexture;
            vertices.push_back(vertex);
        }



        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }


        if (useDiffuseTexture == 1.f)
        {
            std::vector<Texture> diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        }

        return Mesh(vertices, indices, textures, alpha<1.f);
    }

    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
        }
        return textures;
    }
};


unsigned int TextureFromFile(const char* path, const std::string& directory)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Failed to load texture at " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
