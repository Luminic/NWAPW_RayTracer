#include "Settings3D.hpp"

Settings3dWindow::Settings3dWindow(QWidget* parent) : QDockWidget(parent) {
    settings3d = new QDockWidget(tr("3D Rendering Settings"), parent);
    settings3d->setAllowedAreas(Qt::RightDockWidgetArea);
    QListWidget* list = new QListWidget(settings3d);
    // TODO: get settings from respective classes, maybe as an array
    // list.additem("Some setting here")
    resize(800, 600);
    show();
}

Settings3dWindow::~Settings3dWindow() {

}
