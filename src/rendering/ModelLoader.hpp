#ifndef MODEL_LOADER_4D_HPP
#define MODEL_LOADER_4D_HPP

#include <QObject>
#include "objects/Node.hpp"
#include "ModelLoaderData.hpp"

struct Primitive {
    Primitive() = default;
    Primitive(const Index* indices, Index indexCount, Index vertexCount)
        : indices(indices, indices + indexCount), vertexCount(vertexCount) {}
    Primitive& operator=(Primitive&& primitive) noexcept {
        indices = std::move(primitive.indices);
        vertexCount = primitive.vertexCount;
        return *this;
    }

    std::vector<Index> indices;
    Index vertexCount = 0;
};

class ModelLoader : public QObject {
    Q_OBJECT;
public:
    ModelLoader(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~ModelLoader() {}

    Node* load_model(const char* file_path);
};

#endif
