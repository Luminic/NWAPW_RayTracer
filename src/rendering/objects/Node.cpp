#include "Node.hpp"
#include <QDebug>

Node::Node(const std::vector<AbstractMesh*>& meshes, QObject* parent) : QObject(parent), meshes(meshes) {
    transformation = glm::mat4(1.0f);
}

Node::Node(glm::mat4 transformation, QObject* parent) : QObject(parent), transformation(transformation) {}

Node::Node(glm::mat4 transformation, const std::vector<AbstractMesh*>& meshes, QObject* parent) :
    QObject(parent),
    transformation(transformation),
    meshes(meshes)
{}

void Node::add_mesh_data(std::vector<unsigned char>& resulting_mesh_data, glm::mat4 parent_transformation) {
    parent_transformation *= transformation;

    for (auto mesh : meshes) {
        mesh->as_byte_array(resulting_mesh_data.data()+mesh->get_mesh_index()*mesh_size_in_opengl, parent_transformation);
    }

    QList<Node*> child_nodes = findChildren<Node*>(QString(), Qt::FindDirectChildrenOnly);
    for (auto node : child_nodes) {
        node->add_mesh_data(resulting_mesh_data, parent_transformation);
    }
}

/*
The node's child nodes can be found using:
QList<Node*> child_nodes = findChildren<Node*>(QString(), Qt::FindDirectChildrenOnly)
*/
