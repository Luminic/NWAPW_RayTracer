#include "ModelLoader3D.hpp"
#include <QDebug>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>

Node* ModelLoader3D::load_model(const char* file_path) {
    const aiScene* scene = aiImportFile(file_path, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);

    if (!scene) {
        qDebug() << "Failed to load model from:" << file_path;
        qDebug() << "Assimp Error: " << aiGetErrorString();
        return nullptr;
    }


    aiReleaseImport(scene);
    return nullptr;
}
