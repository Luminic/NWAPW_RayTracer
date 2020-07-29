#include "Settings3D.hpp"

Settings3D::Settings3D(QWidget* parent) : QFormLayout(parent) {
    form3D = new QFormLayout(parent);

    QLabel* label = new QLabel("3D Settings");
    form3D->addWidget(label);

}

Settings3D::~Settings3D() {

}
