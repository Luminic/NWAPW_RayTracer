#ifndef ABSTRACT_MESH
#define ABSTRACT_MESH

#include <QObject>
#include "Vertex.hpp"

class AbstractMesh : public QObject {
    Q_OBJECT;
public:
    AbstractMesh(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~AbstractMesh() {}

    int vertex_offset;

    virtual void set_mesh_index(int mesh_index) = 0;
    virtual int get_mesh_index() = 0;

    virtual size_t size_vertices() = 0;
    virtual size_t size_indices() = 0;
    virtual const Vertex* get_vertices() = 0;
    virtual const Index* get_indices() = 0;
protected:
    int mesh_index;
};

#endif
