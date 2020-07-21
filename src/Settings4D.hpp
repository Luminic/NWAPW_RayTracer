#ifndef SETTINGS_4D
#define SETTINGS_4D

#include <QDockWidget>
#include <QString>

class SETTINGS_4D : public QDockWidget {
    // TODO: add list of useful settings
    // TODO: add GUI for said settings

    Q_OBJECT;

public:
    SETTINGS_4D(Qwidget* parent = nullptr);
    ~SETTINGS_4D();
};

#endif
