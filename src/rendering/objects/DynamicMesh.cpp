#include "DynamicMesh.hpp"

DynamicMesh::DynamicMesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, QObject* parent) :
    AbstractMesh(parent),
    vertices(vertices),
    indices(indices)
{}

size_t DynamicMesh::size_vertices() {
    return vertices.size();
}

size_t DynamicMesh::size_indices() {
    return indices.size();
}

const Vertex* DynamicMesh::get_vertices() {
    return vertices.data();
}

const Index* DynamicMesh::get_indices() {
    return indices.data();
}

std::vector<Vertex>& DynamicMesh::modify_vertices() {
    return vertices;
}

std::vector<Index>& DynamicMesh::modify_indices() {
    return indices;
}
