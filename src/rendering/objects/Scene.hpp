#ifndef SCENE_HPP
#define SCENE_HPP

#include <QObject>
#include "StaticMesh.hpp"

class Scene : public QObject {
    Q_OBJECT;

public:
    Scene(QObject* parent=nullptr);

    void add_static_mesh(AbstractMesh* static_mesh);
    const std::vector<AbstractMesh*>& get_static_meshes();
    std::vector<AbstractMesh*>& get_static_meshes_modifiable();

    bool static_meshes_modified(bool set_to_false=false);

private:
    // All AbstractMeshes* must be a converted StaticMesh*
    std::vector<AbstractMesh*> static_meshes;
    bool modified_static_meshes;
};

#endif