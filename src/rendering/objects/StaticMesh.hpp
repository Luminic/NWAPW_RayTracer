#ifndef STATIC_MESH_HPP
#define STATIC_MESH_HPP

#include <QObject>
#include <vector>
#include <glm/glm.hpp>

#include "Vertex.hpp"

class VirtualStaticMesh : public QObject {
    Q_OBJECT;

public:
    VirtualStaticMesh(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~VirtualStaticMesh() {}

    virtual size_t size_vertices() = 0;
    virtual size_t size_indices() = 0;
    virtual const Vertex* get_vertices() = 0;
    virtual const Index* get_indices() = 0;
};

/*
A static mesh cannot modify the positions of its vertices nor change its number of vertices
It may still move around in 3D space using its transformation matrix
*/
template <size_t S_verts, size_t S_inds>
class StaticMesh : public VirtualStaticMesh {
public:
    StaticMesh(Vertex verts[S_verts], Index inds[S_inds], QObject* parent=nullptr);

    size_t size_vertices() override;
    size_t size_indices() override;
    // constexpr functions cannot be virtual until c++20
    // when using a converted StaticMesh->VirtualStaticMesh pointer use
    // size() instead
    constexpr size_t size_vertices_cx();
    constexpr size_t size_indices_cx();

    const Vertex* get_vertices() override;
    const Index* get_indices() override;

    glm::mat4 transformation;

private:
    Vertex vertices[S_verts];
    Index indices[S_inds];
};

#include "StaticMesh.tpp"

#endif