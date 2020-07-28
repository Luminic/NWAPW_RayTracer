#ifndef SETTINGS_3D_HPP
#define SETTINGS_3D_HPP

#include <QFormLayout>
#include <QString>
#include <QLabel>

/* Q Form that hooks into Viewport UI to provide access to 3D render
 * settings.
*/

class Settings3D : public QFormLayout {
    // TODO: add accessors / mutators for settings
    Q_OBJECT;
public:
    Settings3D(QWidget* parent=nullptr);
    virtual ~Settings3D();

    QFormLayout* form3D;
};

#endif
