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