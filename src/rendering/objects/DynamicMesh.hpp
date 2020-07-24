#ifndef DYNAMIC_MESH_HPP
#define DYNAMIC_MESH_HPP

#include <QObject>
#include <glm/glm.hpp>

#include "AbstractMesh.hpp"

/*
A dynamic mesh cannot change its number of vertices but it may modify their positions
*/
template <size_t S_verts, size_t S_inds>
class DynamicMesh : public AbstractMesh {
public:
    DynamicMesh(Vertex verts[S_verts], Index inds[S_inds], QObject* parent=nullptr);

    size_t size_vertices() override;
    size_t size_indices() override;
    // constexpr functions cannot be virtual until c++20
    // when using a converted DynamicMesh->AbstractMesh pointer use
    // size() instead
    constexpr size_t size_vertices_cx();
    constexpr size_t size_indices_cx();

    const Vertex* get_vertices() override;
    const Index* get_indices() override;

    Vertex* modify_vertices();
    Index* modify_indices();

private:
    Vertex vertices[S_verts];
    Index indices[S_inds];
};

#include "DynamicMesh.tpp"

#endif