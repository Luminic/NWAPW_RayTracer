#include "Scene.hpp"
#include "DynamicMesh.hpp"

Scene::Scene(QObject* parent) : QObject(parent) {
    modified_static_meshes = false;
    nr_static_vertices = 0;
    nr_static_indices = 0;

    root_nodes.push_back(new Node(this));
}

MaterialManager& Scene::get_material_manager() {
    return material_manager;
}

void Scene::add_root_node(Node* root_node) {
    add_node_meshes(root_node);
    root_nodes.push_back(root_node);

    QList<Node*> child_nodes = root_node->findChildren<Node*>();
    for (auto node : child_nodes) {
        add_node_meshes(node);
    }
}

bool Scene::remove_root_node(Node* root_node) {
    auto root_node_it = std::find(root_nodes.begin(), root_nodes.end(), root_node);
    if (root_node_it == root_nodes.end()) {
        return false;
    }

    root_nodes.erase(root_node_it);

    remove_node_meshes(root_node);
    root_nodes.push_back(root_node);

    QList<Node*> child_nodes = root_node->findChildren<Node*>();
    for (auto node : child_nodes) {
        remove_node_meshes(node);
    }

    return true;
}

const std::vector<Node*>& Scene::get_root_nodes() {
    return root_nodes;
}

void Scene::add_node_meshes(Node* node) {
    for (auto mesh : node->meshes) {
        if (dynamic_cast<DynamicMesh*>(mesh)) {
            add_dynamic_mesh(mesh, false);
        } else if (mesh) {
            add_static_mesh(mesh, false);
        }
    }
}

void Scene::remove_node_meshes(Node* node) {
    for (auto mesh : node->meshes) {
        if (dynamic_cast<DynamicMesh*>(mesh)) {
            remove_dynamic_mesh(mesh);
        } else if (mesh) {
            remove_static_mesh(mesh);
        }
    }
}

void Scene::add_static_mesh(AbstractMesh* static_mesh, bool orphaned) {
    static_meshes.push_back(static_mesh);
    modified_static_meshes = true;

    if (nr_static_vertices >= 0) nr_static_vertices += static_mesh->size_vertices();
    if (nr_static_indices >= 0) nr_static_indices += static_mesh->size_indices();

    if (orphaned) {
        static_mesh->set_node_parent(root_nodes[0]);
        root_nodes[0]->meshes.push_back(static_mesh);
    }
}

bool Scene::remove_static_mesh(AbstractMesh* static_mesh) {
    auto static_mesh_it = std::find(static_meshes.begin(), static_meshes.end(), static_mesh);
    if (static_mesh_it == static_meshes.end()) {
        return false;
    }
    static_meshes.erase(static_mesh_it);

    static_mesh_it = std::find(root_nodes[0]->meshes.begin(), root_nodes[0]->meshes.end(), static_mesh);
    if (static_mesh_it != root_nodes[0]->meshes.end()) {
        root_nodes[0]->meshes.erase(static_mesh_it);
    }

    if (nr_static_vertices >= 0) nr_static_vertices -= static_mesh->size_vertices();
    if (nr_static_indices >= 0) nr_static_indices -= static_mesh->size_indices();
}

const std::vector<AbstractMesh*>& Scene::get_static_meshes() const {
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

void Scene::add_dynamic_mesh(AbstractMesh* dynamic_mesh, bool orphaned) {
    dynamic_meshes.push_back(dynamic_mesh);
    if (orphaned) {
        dynamic_mesh->set_node_parent(root_nodes[0]);
        root_nodes[0]->meshes.push_back(dynamic_mesh);
    }
}

bool Scene::remove_dynamic_mesh(AbstractMesh* dynamic_mesh) {
    auto dynamic_mesh_it = std::find(dynamic_meshes.begin(), dynamic_meshes.end(), dynamic_mesh);
    if (dynamic_mesh_it == dynamic_meshes.end()) {
        return false;
    }
    dynamic_meshes.erase(dynamic_mesh_it);

    dynamic_mesh_it = std::find(root_nodes[0]->meshes.begin(), root_nodes[0]->meshes.end(), dynamic_mesh);
    if (dynamic_mesh_it != root_nodes[0]->meshes.end()) {
        root_nodes[0]->meshes.erase(dynamic_mesh_it);
    }
}

const std::vector<AbstractMesh*>& Scene::get_dynamic_meshes() const {
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

AbstractMesh* Scene::get_mesh(int mesh_index) const {
    // Easy first check
    if (mesh_index < 0)
        return nullptr;

    int static_mesh_size = (int)static_meshes.size();
    int dynamic_mesh_index = mesh_index - static_mesh_size;

    if (mesh_index < static_mesh_size)
        return static_meshes[mesh_index];
    else if (dynamic_mesh_index < (int)dynamic_meshes.size())
        return dynamic_meshes[dynamic_mesh_index];
    else
        return nullptr;
}
