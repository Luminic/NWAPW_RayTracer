#ifndef SETTINGS_4D
#define SETTINGS_4D

#include <QDockWidget>
#include <QString>

class Settings4dWindow : public QDockWidget {
    // TODO: add list of useful settings
    // TODO: setup .ui file

    Q_OBJECT;

public:
    QDockWidget* settings4d;

    Settings4dWindow(QWidget* parent);
    ~Settings4dWindow();
};

#endif
