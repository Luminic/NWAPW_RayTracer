#ifndef SCENE_HPP
#define SCENE_HPP

#include <QObject>
#include "Node.hpp"
#include "AbstractMesh.hpp"

class Scene : public QObject {
    Q_OBJECT;

public:
    Scene(QObject* parent=nullptr);

    void add_static_root_node(Node* root_node);

    void add_static_mesh(AbstractMesh* static_mesh);
    const std::vector<AbstractMesh*>& get_static_meshes();
    std::vector<AbstractMesh*>& get_static_meshes_modifiable();
    bool static_meshes_modified(bool set_to_false=false);
    int get_nr_static_vertices();
    int get_nr_static_indices();

    void add_dynamic_mesh(AbstractMesh* dynamic_mesh);
    const std::vector<AbstractMesh*>& get_dynamic_meshes();
    std::vector<AbstractMesh*>& get_dynamic_meshes_modifiable();
    int get_nr_dynamic_vertices();
    int get_nr_dynamic_indices();

private:
    std::vector<Node*> root_nodes;
    void add_static_node_meshes(Node* node);

    std::vector<AbstractMesh*> static_meshes;
    bool modified_static_meshes;
    int nr_static_vertices;
    int nr_static_indices;

    std::vector<AbstractMesh*> dynamic_meshes;
};

#endif