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

    std::vector<AbstractMesh*> meshes;
    meshes.reserve(scene->mNumMeshes);
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[i];

        qDebug() << "Mesh" << i << "has" << mesh->mNumVertices << "vertices.";
        qDebug() << "Mesh" << i << "has" << mesh->mNumFaces << "faces.";

        // Get material info
        aiMaterial* meshMaterial = scene->mMaterials[mesh->mMaterialIndex];
        aiColor4D specular, diffuse, ambient;
        float shininess;

        aiGetMaterialColor(meshMaterial, AI_MATKEY_COLOR_SPECULAR, &specular);
        aiGetMaterialColor(meshMaterial, AI_MATKEY_COLOR_DIFFUSE, &diffuse);
        aiGetMaterialColor(meshMaterial, AI_MATKEY_COLOR_AMBIENT, &ambient);
        aiGetMaterialFloat(meshMaterial, AI_MATKEY_SHININESS, &shininess);

        Material material(glm::vec4(specular.r, specular.g, specular.b, shininess),
                          glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a),
                          glm::vec4(ambient.r, ambient.g, ambient.b, ambient.a), this);

        // Get vertices
        std::vector<Vertex> vertices;
        vertices.reserve(mesh->mNumVertices);
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
            aiVector3D mesh_position = mesh->mVertices[j];

            glm::vec4 position(mesh_position.x, mesh_position.y, mesh_position.z, 1.0f);
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

            vertices.push_back(Vertex(position, normal, tex_coords));
        }

        qDebug() << "Mesh" << i << (mesh->HasNormals() ? "has" : "doesn't have") << "normals.";
        qDebug() << "Mesh" << i << (mesh->HasTextureCoords(0) ? "has" : "doesn't have") << "texture coordinates.";

        // Get indices
        // NOTE: Assumes that faces are triangles
        std::vector<Index> indices;
        indices.reserve(mesh->mNumFaces * 3 /* 3 indices per triangle */);
        for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
            assert(mesh->mFaces->mNumIndices == 3);

            // could be a for loop but whatever
            indices.push_back(mesh->mFaces[j].mIndices[0]);
            indices.push_back(mesh->mFaces[j].mIndices[1]);
            indices.push_back(mesh->mFaces[j].mIndices[2]);
        }

        // Form DynamicMesh
        meshes.push_back(new DynamicMesh(vertices, indices, this));
    }

    aiReleaseImport(scene);
    return new Node(meshes, this);
}
