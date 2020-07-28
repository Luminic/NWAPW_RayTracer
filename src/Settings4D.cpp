#include "Settings4D.hpp"

Settings4D::Settings4D(QWidget* parent) : QFormLayout(parent) {
    settings4D = new QFormLayout(parent);

    QLabel* label = new QLabel("4D Settings");
    settings4D->addWidget(label);
}

Settings4D::~Settings4D() {

}
