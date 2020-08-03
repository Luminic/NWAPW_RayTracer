#ifndef SCENE_HPP
#define SCENE_HPP

#include <QObject>
#include "Node.hpp"
#include "AbstractMesh.hpp"
#include "MaterialManager.hpp"

class Scene : public QObject {
    Q_OBJECT;
public:
    Scene(QObject* parent=nullptr);

    MaterialManager& get_material_manager();

    void add_root_node(Node* root_node);
    const std::vector<Node*>& get_root_nodes();

    // Orphaned is true if the mesh is not a part of a node
    void add_static_mesh(AbstractMesh* static_mesh, bool orphaned=true);
    const std::vector<AbstractMesh*>& get_static_meshes() const;
    std::vector<AbstractMesh*>& get_static_meshes_modifiable();
    bool static_meshes_modified(bool set_to_false=false);
    int get_nr_static_vertices();
    int get_nr_static_indices();

    // Orphaned is true if the mesh is not a part of a node
    void add_dynamic_mesh(AbstractMesh* dynamic_mesh, bool orphaned=true);
    const std::vector<AbstractMesh*>& get_dynamic_meshes() const;
    std::vector<AbstractMesh*>& get_dynamic_meshes_modifiable();
    int get_nr_dynamic_vertices();
    int get_nr_dynamic_indices();

    AbstractMesh* get_mesh(int mesh_index) const;
private:
    MaterialManager material_manager;

    // root_nodes[0] is the scene node where orphaned meshes are put
    std::vector<Node*> root_nodes;
    void add_node_meshes(Node* node);

    std::vector<AbstractMesh*> static_meshes;
    bool modified_static_meshes;
    int nr_static_vertices;
    int nr_static_indices;

    std::vector<AbstractMesh*> dynamic_meshes;
};

#endif
