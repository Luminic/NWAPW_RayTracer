#ifndef MODEL_LOADER_4D_HPP
#define MODEL_LOADER_4D_HPP

#include <QObject>
#include "objects/Node.hpp"

struct vert {
    unsigned int position_index;
    unsigned int normal_index;
    unsigned int tex_coord_index;

    bool operator==(const vert& other) {
        return position_index == other.position_index &&
               normal_index == other.normal_index &&
               tex_coord_index == other.tex_coord_index;
    }
};

class ModelLoader4D : public QObject {
    Q_OBJECT;
public:
    ModelLoader4D(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~ModelLoader4D() {}

    Node* load_model(const char* file_path);
};

#endif
