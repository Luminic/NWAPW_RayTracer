#ifndef ABSTRACT_MESH
#define ABSTRACT_MESH

#include <QObject>
#include <glm/glm.hpp>
#include "Vertex.hpp"
#include "Material.hpp"

constexpr int mesh_size_in_opengl = 80;

class AbstractMesh : public QObject {
    Q_OBJECT;
public:
    AbstractMesh(QObject* parent=nullptr);
    virtual ~AbstractMesh() {}

    int vertex_offset;
    int material_index;

    virtual void set_mesh_index(int mesh_index) = 0;
    virtual int get_mesh_index() const = 0;

    virtual size_t size_vertices() const = 0;
    virtual size_t size_indices() const = 0;
    virtual const Vertex* get_vertices() const = 0;
    virtual const Index* get_indices() const = 0;

    void as_byte_array(unsigned char byte_array[mesh_size_in_opengl], const glm::mat4& transformation) const;

protected:
    int mesh_index;
};

#endif
