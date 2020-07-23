#ifndef MODEL_LOADER_3D_HPP
#define MODEL_LOADER_3D_HPP

#include <QObject>
#include "objects/Node.hpp"

class ModelLoader3D : public QObject {
    Q_OBJECT;
public:
    Node* load_model(const char* file_path);
};

#endif
