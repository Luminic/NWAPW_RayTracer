#ifndef SETTINGS_4D_HPP
#define SETTINGS_4D_HPP

#include <QFormLayout>
#include <QString>
#include <QLabel>

/* Q Form that hooks into Viewport UI to provide access to 4D render
 * settings.
*/

class Settings4D : public QFormLayout {
    // TODO: add accessors / mutators for settings
    Q_OBJECT;
public:
    Settings4D(QWidget* parent=nullptr);
    virtual ~Settings4D();

    QFormLayout* settings4D;
};

#endif
