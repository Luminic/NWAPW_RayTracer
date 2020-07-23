#include "Scene.hpp"

Scene::Scene(QObject* parent) : QObject(parent) {
    modified_static_meshes = false;
}

void Scene::add_static_mesh(AbstractMesh* static_mesh) {
    static_meshes.push_back(static_mesh);
    modified_static_meshes = true;
}

const std::vector<AbstractMesh*>& Scene::get_static_meshes() {
    return static_meshes;
}

std::vector<AbstractMesh*>& Scene::get_static_meshes_modifiable() {
    modified_static_meshes = true;
    return static_meshes;
}

bool Scene::static_meshes_modified(bool set_to_false) {
    bool tmp = modified_static_meshes;
    if (set_to_false) {
        modified_static_meshes = false;
    }
    return tmp;
}