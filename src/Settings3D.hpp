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
    Settings3dWindow(QWidget* parent=nullptr);
    virtual ~Settings3dWindow();

    QDockWidget* settings3d;
};

#endif
