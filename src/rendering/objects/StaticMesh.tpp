#include "algorithm"

template <size_t S_verts, size_t S_inds>
void StaticMesh<S_verts, S_inds>::set_mesh_index(int mesh_index) {
    if (this->mesh_index != mesh_index) {
        this->mesh_index = mesh_index;

        for (size_t i=0; i<S_verts; i++) {
            vertices[i].mesh_index = mesh_index;
        }
    }
}

template <size_t S_verts, size_t S_inds>
int StaticMesh<S_verts, S_inds>::get_mesh_index() {
    return mesh_index;
}

template <size_t S_verts, size_t S_inds>
StaticMesh<S_verts, S_inds>::StaticMesh(Vertex verts[S_verts], Index inds[S_inds], QObject* parent) : AbstractMesh(parent) {
    std::copy(verts, verts+S_verts, vertices);
    std::copy(inds, inds+S_inds, indices);
}

template <size_t S_verts, size_t S_inds>
size_t StaticMesh<S_verts, S_inds>::size_vertices() {
    return S_verts;
}

template <size_t S_verts, size_t S_inds>
size_t StaticMesh<S_verts, S_inds>::size_indices() {
    return S_inds;
}

template <size_t S_verts, size_t S_inds>
constexpr size_t StaticMesh<S_verts, S_inds>::size_vertices_cx() {
    return S_verts;
}

template <size_t S_verts, size_t S_inds>
constexpr size_t StaticMesh<S_verts, S_inds>::size_indices_cx() {
    return S_inds;
}

template <size_t S_verts, size_t S_inds>
const Vertex* StaticMesh<S_verts, S_inds>::get_vertices() {
    return vertices;
}

template <size_t S_verts, size_t S_inds>
const Index* StaticMesh<S_verts, S_inds>::get_indices() {
    return indices;
}
