#include "Scene.hpp"
#include "DynamicMesh.hpp"

Scene::Scene(QObject* parent) : QObject(parent) {
    modified_static_meshes = false;
    nr_static_vertices = 0;
    nr_static_indices = 0;
}

void Scene::add_root_node(Node* root_node) {
    add_node_meshes(root_node);
    root_nodes.push_back(root_node);

    QList<Node*> child_nodes = root_node->findChildren<Node*>();
    for (auto node : child_nodes) {
        add_node_meshes(node);
    }
}

const std::vector<Node*>& Scene::get_root_nodes() {
    return root_nodes;
}

void Scene::add_node_meshes(Node* node) {
    for (auto mesh : node->meshes) {
        if (dynamic_cast<DynamicMesh*>(mesh)) {
            add_dynamic_mesh(mesh);
        } else if (mesh) {
            add_static_mesh(mesh);
        }
    }
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
        nr_static_vertices = 0;
        for (auto mesh : static_meshes) {
            nr_static_vertices += mesh->size_vertices();
        }
    }
    return nr_static_vertices;
}

int Scene::get_nr_static_indices() {
    if (nr_static_indices == -1) {
        nr_static_indices = 0;
        for (auto mesh : static_meshes) {
            nr_static_indices += mesh->size_indices();
        }
    }
    return nr_static_indices;
}

void Scene::add_dynamic_mesh(AbstractMesh* dynamic_mesh) {
    dynamic_meshes.push_back(dynamic_mesh);
}

const std::vector<AbstractMesh*>& Scene::get_dynamic_meshes() {
    return dynamic_meshes;
}

std::vector<AbstractMesh*>& Scene::get_dynamic_meshes_modifiable() {
    return dynamic_meshes;
}

int Scene::get_nr_dynamic_vertices() {
    int nr_dynamic_vertices = 0;
    for (auto mesh : dynamic_meshes) {
        nr_dynamic_vertices += mesh->size_vertices();
    }
    return nr_dynamic_vertices;
}

int Scene::get_nr_dynamic_indices() {
    int nr_dynamic_indices = 0;
    for (auto mesh : dynamic_meshes) {
        nr_dynamic_indices += mesh->size_indices();
    }
    return nr_dynamic_indices;
}
