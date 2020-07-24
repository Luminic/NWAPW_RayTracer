#ifndef SCENE_HPP
#define SCENE_HPP

#include <QObject>
#include "StaticMesh.hpp"
#include "DynamicMesh.hpp"

class Scene : public QObject {
    Q_OBJECT;

public:
    Scene(QObject* parent=nullptr);

    int get_nr_vertices();
    int get_nr_indices();

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
    // All AbstractMeshes* in static_meshes must be a converted StaticMesh*
    std::vector<AbstractMesh*> static_meshes;
    bool modified_static_meshes;
    int nr_static_vertices;
    int nr_static_indices;
    void update_nr_static_vi();

    std::vector<AbstractMesh*> dynamic_meshes;
    int nr_dynamic_vertices;
    int nr_dynamic_indices;
    void update_nr_dynamic_vi();
};

#endif