#ifndef CAMERA_CONTROLLER
#define CAMERA_CONTROLLER

#include <QObject>
#include <QKeyEvent>

#include "rendering/Camera3D.hpp"

struct Movement {
    bool front;
    bool back;

    bool left;
    bool right;

    bool up;
    bool down;

    void reset() {
        front = false;
        back = false;
        left = false;
        right = false;
        up = false;
        down = false;
    }
};

class CameraController : public QObject{
    Q_OBJECT;
public:
    CameraController(float mouse_sensitivity=3.0f, float speed=3.0f, QObject* parent=nullptr);
    virtual ~CameraController() {}

    void main_loop(float dt);

    void mouse_moved(float dx, float dy);
    void key_event(QKeyEvent* key);
    void update_fov(float fov_change);

    void set_camera_3D(Camera3D* camera_3D);
private:
    Camera3D* camera_3D;

    float mouse_sensitivity;
    float speed;

    glm::vec2 mouse_movement;
    Movement movement;
};

#endif
