#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

#include "ui_Viewport.h"

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

    float return_slider4D_val();

signals:
    void opengl_initialized();

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void on_iterativeRenderCheckBox_toggled(bool checked);

    void on_fileButton_clicked();

    void on_unused4DSlider_sliderMoved(int position);

private:
    void capture_mouse();
    void release_mouse();
    bool mouse_captured;

    OpenGLWidget gl_widget;
    Renderer3D renderer_3D;
    Camera3D camera_3D;

    CameraController cam_controller;

    Scene* scene;
    Ui::Viewport ui;

    Settings3D* settings3D;
    QLabel* modelLabel;
    QString new_model_path;

    float slider4Dvalue;
};

#endif
