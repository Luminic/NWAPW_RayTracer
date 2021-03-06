#include "DynamicMesh.hpp"

DynamicMesh::DynamicMesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, QObject* parent) :
    AbstractMesh(parent),
    vertices(vertices),
    indices(indices)
{}

void DynamicMesh::set_mesh_index(int mesh_index) {
    this->mesh_index = mesh_index;
    for (unsigned int i=0; i<vertices.size(); i++) {
        vertices[i].mesh_index = mesh_index;
    }
}

int DynamicMesh::get_mesh_index() const {
    return mesh_index;
}

size_t DynamicMesh::size_vertices() const {
    return vertices.size();
}

size_t DynamicMesh::size_indices() const {
    return indices.size();
}

const Vertex* DynamicMesh::get_vertices() const {
    return vertices.data();
}

const Index* DynamicMesh::get_indices() const {
    return indices.data();
}

std::vector<Vertex>& DynamicMesh::modify_vertices() {
    return vertices;
}

std::vector<Index>& DynamicMesh::modify_indices() {
    return indices;
}
