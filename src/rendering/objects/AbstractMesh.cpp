#include "AbstractMesh.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <QDebug>

AbstractMesh::AbstractMesh(QObject* parent) : QObject(parent) {}

void AbstractMesh::as_byte_array(unsigned char byte_array[mesh_size_in_opengl], const glm::mat4& transformation) const {
    unsigned char const* tmp = reinterpret_cast<unsigned char const*>(glm::value_ptr(transformation));
    std::copy(tmp, tmp+64, byte_array);

    uint32_t mat_ind = (uint32_t) material_index;
    tmp = reinterpret_cast<unsigned char const*>(&mat_ind);
    std::copy(tmp, tmp+4, byte_array+64);
}