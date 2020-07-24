#ifndef CAMERA_3D
#define CAMERA_3D

#include <QObject>

#include <glm/glm.hpp>

struct CornerRays {
    glm::vec3 r00;
    glm::vec3 r10;
    glm::vec3 r01;
    glm::vec3 r11;
};

struct CameraDirectionVectors {
    glm::vec3 front;
    glm::vec3 right;
    glm::vec3 up;
};

class Camera3D : public QObject {
    Q_OBJECT;
public:
    glm::vec3 position;
    glm::vec3 yaw_pitch_roll;

    Camera3D(float aspect_ratio=1.0f, float fov=45.0f, QObject* parent=nullptr);
    virtual ~Camera3D() {}

    void update_perspective_matrix(float new_aspect_ratio=0.0f, float new_fov=0.0f);
    void update_view_matrix();
    
    CameraDirectionVectors get_camera_direction_vectors();
    CornerRays get_corner_rays();
private:
    float aspect_ratio;
    float fov;

    glm::mat4 perspective;
    glm::mat4 view;
};

#endif
