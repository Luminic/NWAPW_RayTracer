#include "Vertex.hpp"
#include <algorithm>

Vertex::Vertex(glm::vec4 position, glm::vec4 normal, glm::vec2 tex_coords, int mesh_index) : 
    position(position),
    normal(normal),
    tex_coords(tex_coords),
    mesh_index(mesh_index)
{}

void Vertex::as_byte_array(unsigned char byte_array[48]) const {
    static_assert(sizeof(glm::vec4) == 16, "Vertex overflow");
    static_assert(sizeof(glm::vec2) == 8, "Vertex overflow");

    unsigned char const* tmp = reinterpret_cast<unsigned char const*>(&position);
    std::copy(tmp, tmp+16, byte_array);

    tmp = reinterpret_cast<unsigned char const*>(&normal);
    std::copy(tmp, tmp+16, byte_array+16);

    tmp = reinterpret_cast<unsigned char const*>(&tex_coords);
    std::copy(tmp, tmp+8, byte_array+32);

    tmp = reinterpret_cast<unsigned char const*>(&mesh_index);
    std::copy(tmp, tmp+4, byte_array+40);
}
