#include "Viewport.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QGridLayout>
#include <QDebug>

Viewport::Viewport(QWidget* parent) : QWidget(parent), gl_widget(this), renderer_3D(this) {
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&gl_widget, 0, 0);
    setFocusPolicy(Qt::StrongFocus);

    renderer_3D.set_camera(&camera_3D);
    gl_widget.set_renderer(&renderer_3D);
}

Viewport::~Viewport() {}

void Viewport::main_loop() {
    gl_widget.main_loop();
}