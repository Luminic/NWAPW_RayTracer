#ifndef SETTINGS_3D_HPP
#define SETTINGS_3D_HPP

#include <QDockWidget>
#include <QString>
#include <QListWidget>

#include "Viewport.hpp"

class Settings3dWindow : public QDockWidget {
    // TODO: add accessors / mutators for settings
    // TODO: setup .ui file
    Q_OBJECT;

public:
    QDockWidget* settings3d;

    Settings3dWindow(QWidget* parent);
    ~Settings3dWindow();
};

#endif
