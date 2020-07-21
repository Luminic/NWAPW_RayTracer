#ifndef SETTINGS_3D_HPP
#define SETTINGS_3D_hpp

#include <QDockWidget>
#include <QString>

class Settings3DWindow : public QDockWidget {
    // TODO: add list of useful settings
    // TODO: add GUI for said settings

    Q_OBJECT;

public:
    Settings3DWindow(QWidget* parent = nullptr);
    ~Settings3DWindow();
};

#endif
