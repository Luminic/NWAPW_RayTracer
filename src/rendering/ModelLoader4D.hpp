#ifndef MODEL_LOADER_4D_HPP
#define MODEL_LOADER_4D_HPP

#include <QObject>
#include "objects/Node.hpp"

struct vert {
    unsigned int position_index;
    unsigned int normal_index;
    unsigned int tex_coord_index;
};

bool operator==(const vert& vert1, const vert& vert2);

class ModelLoader4D : public QObject {
    Q_OBJECT;
public:
    ModelLoader4D(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~ModelLoader4D() {}

    Node* load_model(const char* file_path);
private:
    void store_mesh(const std::vector<vert>& verts,
                    const std::vector<Index>& indices,
                    const std::vector<glm::vec4>& positions,
                    const std::vector<glm::vec4>& normals,
                    const std::vector<glm::vec2>& tex_coords,
                    std::vector<AbstractMesh*>& meshes);
};

#endif
