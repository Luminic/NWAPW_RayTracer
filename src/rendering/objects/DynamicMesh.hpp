#ifndef DYNAMIC_MESH_HPP
#define DYNAMIC_MESH_HPP

#include <QObject>
#include <glm/glm.hpp>
#include <vector>

#include "AbstractMesh.hpp"

/*
A dynamic mesh can change its number of vertices and modify their positions
*/
class DynamicMesh : public AbstractMesh {
public:
    DynamicMesh(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, QObject* parent=nullptr);

    void set_mesh_index(int mesh_index) override;
    int get_mesh_index() const override;

    size_t size_vertices() const override;
    size_t size_indices() const override;
    
    const Vertex* get_vertices() const override;
    const Index* get_indices() const override;

    std::vector<Vertex>& modify_vertices();
    std::vector<Index>& modify_indices();
private:
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
};

#endif
