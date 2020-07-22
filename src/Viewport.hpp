#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include <QWidget>

#include "CameraController.hpp"
#include "rendering/OpenGLWidget.hpp"
#include "rendering/Renderer3D.hpp"
#include "rendering/objects/Scene.hpp"

class Viewport : public QWidget {
    Q_OBJECT;

public:
    Viewport(QWidget* parent=nullptr);
    ~Viewport();

    void main_loop(float dt);
    void set_scene(Scene* scene);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

private:
    void capture_mouse();
    void release_mouse();
    bool mouse_captured;

    OpenGLWidget gl_widget;
    Renderer3D renderer_3D;
    Camera3D camera_3D;

    CameraController cam_controller;

    Scene* scene;
};

#endif
