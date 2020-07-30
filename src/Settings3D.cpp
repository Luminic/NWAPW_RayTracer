#include "Settings3D.hpp"

Settings3D::Settings3D(QObject* parent) : QObject(parent) {

}

Settings3D::~Settings3D() {

}

void Settings3D::toggle_iterative_rendering(bool toggle, Renderer3DOptions* options3D) {
    if (toggle)
        options3D->begin_iterative_rendering();
    else options3D->end_iterative_rendering();
}
