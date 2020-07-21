#ifndef CAMERA_3D
#define CAMERA_3D

#include <QObject>

class Camera3D : public QObject {
    Q_OBJECT;

public:
    Camera3D(QObject* parent=nullptr);
};

#endif