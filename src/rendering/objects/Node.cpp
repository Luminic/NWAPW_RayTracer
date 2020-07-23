#include "Node.hpp"

Node::Node(const std::vector<AbstractMesh*>& meshes, QObject* parent) :
    QObject(parent),
    meshes(meshes)
{
    transformation = glm::mat4(1.0f);
}

Node::Node(glm::mat4 transformation, QObject* parent) :
    QObject(parent),
    transformation(transformation)
{}

Node::Node(glm::mat4 transformation, const std::vector<AbstractMesh*>& meshes, QObject* parent) :
    QObject(parent),
    transformation(transformation),
    meshes(meshes)
{}

/*
The node's child nodes can be found using:
QList<Node*> child_nodes = findChildren<Node*>(QString(), Qt::FindDirectChildrenOnly)
*/