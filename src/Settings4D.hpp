#ifndef SETTINGS_4D
#define SETTINGS_4D

#include <QDockWidget>
#include <QString>

class Settings_4D : public QDockWidget {
    // TODO: add list of useful settings
    // TODO: add GUI for said settings
    Q_OBJECT;
public:
    Settings_4D(QWidget* parent=nullptr);
    virtual ~Settings_4D();
};

#endif
