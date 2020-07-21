#ifndef CAMERA_CONTROLLER
#define CAMERA_CONTROLLER

#include <QObject>

class CameraController : public QObject{
    Q_OBJECT;

public:
    CameraController(QObject* parent=nullptr);
};

#endif