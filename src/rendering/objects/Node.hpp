#ifndef NODE_HPP
#define NODE_HPP

#include <QObject>
#include <vector>
#include <glm/glm.hpp>

#include "AbstractMesh.hpp"

class Node : public QObject {
    Q_OBJECT;
public:
    Node(const std::vector<AbstractMesh*>& meshes, QObject* parent=nullptr);
    Node(glm::mat4 transformation=glm::mat4(1.0f), QObject* parent=nullptr);
    Node(glm::mat4 transformation, const std::vector<AbstractMesh*>& meshes, QObject* parent=nullptr);

    glm::mat4 transformation;
    std::vector<AbstractMesh*> meshes;
};

#endif
