#include "Viewport.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QGridLayout>
#include <QDebug>

Viewport::Viewport(QWidget* parent) : QWidget(parent), gl_widget(this) {
    QGridLayout* layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&gl_widget, 0, 0);
    setFocusPolicy(Qt::StrongFocus);}

Viewport::~Viewport() {}

void Viewport::main_loop() {}