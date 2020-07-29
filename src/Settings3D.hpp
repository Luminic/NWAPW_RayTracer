#ifndef SETTINGS_3D_HPP
#define SETTINGS_3D_HPP

/*
 * Class that manages settings pertaining to 3D renderer.
 * Should interface with friend classes containing settable
 * properties.
*/

#include <QObject>

#include "rendering/Renderer3DOptions.hpp"

class Settings3D : public QObject {
    // TODO: add accessors / mutators for settings
    Q_OBJECT;
public:
    Settings3D(QObject* parent=nullptr);
    virtual ~Settings3D();


    void toggle_iterative_rendering(bool toggle, Renderer3DOptions* options3D);

private:
};

#endif
