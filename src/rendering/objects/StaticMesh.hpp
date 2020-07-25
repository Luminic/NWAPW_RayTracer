#ifndef STATIC_MESH_HPP
#define STATIC_MESH_HPP

#include <QObject>
#include <glm/glm.hpp>

#include "AbstractMesh.hpp"

/*
A static mesh cannot modify the positions of its vertices nor change its number of vertices
*/
template <size_t S_verts, size_t S_inds>
class StaticMesh : public AbstractMesh {
public:
    StaticMesh(Vertex verts[S_verts], Index inds[S_inds], QObject* parent=nullptr);

    void set_mesh_index(int mesh_index) override;
    int get_mesh_index() const override;

    size_t size_vertices() const override;
    size_t size_indices() const override;
    // constexpr functions cannot be virtual until c++20
    // when using a converted StaticMesh->AbstractMesh pointer use
    // size() instead
    constexpr size_t size_vertices_cx() const;
    constexpr size_t size_indices_cx() const;

    const Vertex* get_vertices() const override;
    const Index* get_indices() const override;
private:
    Vertex vertices[S_verts];
    Index indices[S_inds];
};

#include "StaticMesh.tpp"

#endif
