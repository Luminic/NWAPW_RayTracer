#include "ModelLoader3D.hpp"
#include <QDebug>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <vector>
#include "objects/Vertex.hpp"
#include "objects/Material.hpp"
#include "objects/DynamicMesh.hpp"

constexpr size_t MAX_VERTICES = 24; //  1M
constexpr size_t MAX_INDICES = 12;  // 10M

Node* ModelLoader3D::load_model(const char* file_path) {
    const aiScene* scene = aiImportFile(file_path, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene) {
        qDebug() << "Failed to load model from:" << file_path;
        qDebug() << "Assimp Error: " << aiGetErrorString();
        return nullptr;
    } else if (scene->mNumMeshes == 0) {
        qDebug() << "No meshes found in model:" << file_path;
        aiReleaseImport(scene);
        return nullptr;
    }

    std::vector<AbstractMesh*> meshes(scene->mNumMeshes);
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];

        if (mesh->mNumVertices > MAX_VERTICES) {
            qDebug() << "Vertex count:" << mesh->mNumVertices << "exceeds maximum number of vertices" << MAX_VERTICES << "in model:" << file_path;
            aiReleaseImport(scene);
            return nullptr;
        } else {
            qDebug() << "Mesh" << i << "has" << mesh->mNumVertices << "vertices.";
        }

        if (mesh->mNumFaces > MAX_INDICES) {
            qDebug() << "Index count:" << mesh->mNumFaces << "exceeds maximum number of indices:" << MAX_INDICES << "in model:" << file_path;
            aiReleaseImport(scene);
            return nullptr;
        } else {
            qDebug() << "Mesh" << i << "has" << mesh->mNumFaces << "indices.";
        }

        // Get material info
        aiMaterial* meshMaterial = scene->mMaterials[mesh->mMaterialIndex];
        aiColor4D specular, diffuse, ambient;
        float shininess;

        aiGetMaterialColor(meshMaterial, AI_MATKEY_COLOR_SPECULAR, &specular);
        aiGetMaterialColor(meshMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
        aiGetMaterialColor(meshMaterial, AI_MATKEY_COLOR_AMBIENT, &ambient);
        aiGetMaterialFloat(meshMaterial, AI_MATKEY_COLOR_SPECULAR, &shininess);

        Material material(glm::vec4(specular.r, specular.g, specular.b, shininess),
                          glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a),
                          glm::vec4(ambient.r, ambient.g, ambient.b, ambient.a), this);

        // Get vertices
        unsigned int vertexCount = 0;
        std::vector<Vertex> vertices(MAX_VERTICES); // mesh->mNumVertices
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D mesh_position = mesh->mVertices[j];

            glm::vec4 position(mesh_position.x, mesh_position.y, mesh_position.z, 0.0f);
            glm::vec4 normal(0.0f);
            glm::vec2 tex_coords(0.0f);

            if (mesh->HasNormals()) {
                aiVector3D mesh_normal = mesh->mNormals[j];
                normal = glm::vec4(mesh_normal.x, mesh_normal.y, mesh_normal.z, 0.0f);
            }

            // TODO: assumes there is only one texture coord per vertex
            // there is a min of 0 and a max of AI_MAX_NUMBER_OF_TEXTURECOORDS (currently 8)
            if (mesh->HasTextureCoords(0)) {
                // TODO: this also assumes that the texture coords are 2D
                aiVector3D mesh_tex_coords = mesh->mTextureCoords[0][j];
                tex_coords = glm::vec2(mesh_tex_coords.x, mesh_tex_coords.y);
            }

            vertices[vertexCount++] = Vertex(position, normal, tex_coords);
        }

        qDebug() << "Mesh" << i << (mesh->HasNormals() ? "has" : "doesn't have") << "normals.";
        qDebug() << "Mesh" << i << (mesh->HasTextureCoords(0) ? "has" : "doesn't have") << "texture coordinates.";

        // Get indices
        unsigned int indexCount = 0;
        std::vector<Index> indices(MAX_INDICES); // mesh->mNumFaces * 3 /* 3 indices per triangle */
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            assert(mesh->mFaces->mNumIndices == 3);

            // could be a for loop but whatever
            indices[indexCount++] = mesh->mFaces->mIndices[0];
            indices[indexCount++] = mesh->mFaces->mIndices[1];
            indices[indexCount++] = mesh->mFaces->mIndices[2];
        }

        // Form DynamicMesh
        meshes.push_back((new DynamicMesh(vertices, indices)));
    }

    aiReleaseImport(scene);
    return new Node(meshes, this);
}
