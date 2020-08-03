#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

// #include "ui_Viewport.h"

#include <QWidget>
#include <QApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QString>
#include <QGridLayout>
#include <QDebug>
#include <QtUiTools>
#include <QDesktopServices>
#include <QFileDialog>

#include "CameraController.hpp"
#include "rendering/OpenGLWidget.hpp"
#include "rendering/Renderer3D.hpp"
#include "rendering/Renderer3DOptions.hpp"
#include "rendering/objects/Scene.hpp"

#include "Settings3D.hpp"
#include "Settings4D.hpp"


class Viewport : public QWidget {
    Q_OBJECT;
public:
    Viewport(QWidget* parent=nullptr);
    virtual ~Viewport() {}

    void main_loop(float dt);
    void set_scene(Scene* scene);

    Renderer3DOptions* get_renderer_3D_options();
    void reset_pressed();
    bool is_mouse_pressed() const;

    int get_selected_mesh_index();

signals:
    void opengl_initialized();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    void capture_mouse();
    void release_mouse();
    bool mouse_captured;
    bool file_changed;

    OpenGLWidget gl_widget;
    Renderer3D renderer_3D;
    Camera3D camera_3D;

    CameraController cam_controller;

    Scene* scene;

    bool mouse_pressed = false;
};

#endif
