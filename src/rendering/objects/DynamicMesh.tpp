#include "algorithm"

template <size_t S_verts, size_t S_inds>
DynamicMesh<S_verts, S_inds>::DynamicMesh(Vertex verts[S_verts], Index inds[S_inds], QObject* parent) : AbstractMesh(parent) {
    std::copy(verts, verts+S_verts, vertices);
    std::copy(inds, inds+S_inds, indices);
}

template <size_t S_verts, size_t S_inds>
size_t DynamicMesh<S_verts, S_inds>::size_vertices() {
    return S_verts;
}

template <size_t S_verts, size_t S_inds>
size_t DynamicMesh<S_verts, S_inds>::size_indices() {
    return S_inds;
}

template <size_t S_verts, size_t S_inds>
constexpr size_t DynamicMesh<S_verts, S_inds>::size_vertices_cx() {
    return S_verts;
}

template <size_t S_verts, size_t S_inds>
constexpr size_t DynamicMesh<S_verts, S_inds>::size_indices_cx() {
    return S_inds;
}

template <size_t S_verts, size_t S_inds>
const Vertex* DynamicMesh<S_verts, S_inds>::get_vertices() {
    return vertices;
}

template <size_t S_verts, size_t S_inds>
const Index* DynamicMesh<S_verts, S_inds>::get_indices() {
    return indices;
}

template <size_t S_verts, size_t S_inds>
Vertex* DynamicMesh<S_verts, S_inds>::modify_vertices() {
    return vertices;
}

template <size_t S_verts, size_t S_inds>
Index* DynamicMesh<S_verts, S_inds>::modify_indices() {
    return indices;
}
