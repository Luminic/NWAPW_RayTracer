#ifndef MODEL_LOADER_4D_HPP
#define MODEL_LOADER_4D_HPP

#include <QObject>
#include "objects/Node.hpp"

class ModelLoader4D : public QObject {
    Q_OBJECT;
public:
    ModelLoader4D(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~ModelLoader4D() {}

    Node* load_model(const char* file_path);
};

#endif
