#include "Renderer3DOptions.hpp"

Renderer3DOptions::Renderer3DOptions(Renderer3D* renderer_3D, QObject* parent) :
    QObject(parent),
    renderer_3D(renderer_3D)
{}

void Renderer3DOptions::begin_iterative_rendering() {
    renderer_3D->begin_iterative_rendering();
}

void Renderer3DOptions::end_iterative_rendering() {
    renderer_3D->end_iterative_rendering();
}

bool Renderer3DOptions::modify_sunlight(const glm::vec3& direction, const glm::vec3& radiance, float ambient_multiplier) {
    return renderer_3D->modify_sunlight(direction, radiance, ambient_multiplier);
}

MeshIndex Renderer3DOptions::get_mesh_index_at(int x, int y) {
    return renderer_3D->get_mesh_index_at(x, y);
}