#include "Scene.hpp"

#include <QDebug>

Scene::Scene(QObject* parent) : QObject(parent) {
    modified_static_meshes = false;
    nr_static_vertices = 0;
    nr_static_indices = 0;
    nr_dynamic_vertices = 0;
    nr_dynamic_indices = 0;
}

int Scene::get_nr_vertices() {
    return get_nr_static_vertices() + get_nr_dynamic_vertices();
}

int Scene::get_nr_indices() {
    return get_nr_static_indices() + get_nr_dynamic_indices();
}


void Scene::add_static_mesh(AbstractMesh* static_mesh) {
    static_meshes.push_back(static_mesh);
    modified_static_meshes = true;

    if (nr_static_vertices >= 0) nr_static_vertices += static_mesh->size_vertices();
    if (nr_static_indices >= 0) nr_static_indices += static_mesh->size_indices();
}

const std::vector<AbstractMesh*>& Scene::get_static_meshes() {
    return static_meshes;
}

std::vector<AbstractMesh*>& Scene::get_static_meshes_modifiable() {
    modified_static_meshes = true;
    nr_static_vertices = -1;
    nr_static_indices = -1;
    return static_meshes;
}

bool Scene::static_meshes_modified(bool set_to_false) {
    bool tmp = modified_static_meshes;
    if (set_to_false) {
        modified_static_meshes = false;
    }
    return tmp;
}

int Scene::get_nr_static_vertices() {
    if (nr_static_vertices == -1) {
        update_nr_static_vi();
    }
    return nr_static_vertices;
}

int Scene::get_nr_static_indices() {
    if (nr_static_indices == -1) {
        update_nr_static_vi();
    }
    return nr_static_indices;
}

void Scene::update_nr_static_vi() {
    nr_static_vertices = 0;
    nr_static_indices = 0;
    for (AbstractMesh* static_mesh : static_meshes) {
        nr_static_vertices += static_mesh->size_vertices();
        nr_static_indices += static_mesh->size_indices();
    }
}


void Scene::add_dynamic_mesh(AbstractMesh* dynamic_mesh) {
    dynamic_meshes.push_back(dynamic_mesh);

    if (nr_dynamic_vertices >= 0) nr_dynamic_vertices += dynamic_mesh->size_vertices();
    if (nr_dynamic_indices >= 0) nr_dynamic_indices += dynamic_mesh->size_indices();
}

const std::vector<AbstractMesh*>& Scene::get_dynamic_meshes() {
    return dynamic_meshes;
}

std::vector<AbstractMesh*>& Scene::get_dynamic_meshes_modifiable() {
    nr_dynamic_vertices = -1;
    nr_dynamic_vertices = -1;
    return dynamic_meshes;
}

int Scene::get_nr_dynamic_vertices() {
    if (nr_dynamic_vertices == -1) {
        update_nr_dynamic_vi();
    }
    return nr_dynamic_vertices;
}

int Scene::get_nr_dynamic_indices() {
    if (nr_dynamic_vertices == -1) {
        update_nr_dynamic_vi();
    }
    return nr_dynamic_indices;
}

void Scene::update_nr_dynamic_vi() {
    nr_dynamic_vertices = 0;
    nr_dynamic_indices = 0;
    for (AbstractMesh* dynamic_mesh : dynamic_meshes) {
        nr_dynamic_vertices += dynamic_mesh->size_vertices();
        nr_dynamic_indices += dynamic_mesh->size_indices();
    }
}