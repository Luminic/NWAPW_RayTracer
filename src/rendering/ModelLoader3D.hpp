#ifndef MODEL_LOADER_3D_HPP
#define MODEL_LOADER_3D_HPP

#include <QObject>
#include "objects/Node.hpp"

class ModelLoader3D : public QObject {
    Q_OBJECT;
public:
    ModelLoader3D(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~ModelLoader3D() {}

    Node* load_model(const char* file_path);
};

#endif
