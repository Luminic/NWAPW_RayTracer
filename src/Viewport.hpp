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

    // Getters and setters
    float return_slider4D_val();
    bool return_file_changed();
    void set_file_changed(bool);
    inline float getXSlider()  const { return xSliderValue;  }
    inline float getYSlider()  const { return ySliderValue;  }
    inline float getZSlider()  const { return zSliderValue;  }
    inline float getXWSlider() const { return xwSliderValue; }
    inline float getYWSlider() const { return ywSliderValue; }
    inline float getZWSlider() const { return zwSliderValue; }

    QString get_new_model_path() const;

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

    void on_slice4DSlider_sliderMoved(int position);

    // all of these sliders have the range [0,3600]
    inline void  on_rotateXSlider_sliderMoved(int position) { xSliderValue  = position / 10.0f; }
    inline void  on_rotateYSlider_sliderMoved(int position) { ySliderValue  = position / 10.0f; }
    inline void  on_rotateZSlider_sliderMoved(int position) { zSliderValue  = position / 10.0f; }
    inline void on_rotateXWSlider_sliderMoved(int position) { xwSliderValue = position / 10.0f; }
    inline void on_rotateYWSlider_sliderMoved(int position) { ywSliderValue = position / 10.0f; }
    inline void on_rotateZWSlider_sliderMoved(int position) { zwSliderValue = position / 10.0f; }

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
    Ui::Viewport ui;

    Settings3D* settings3D;
    QLabel* modelLabel;
    QString new_model_path;

    float slider4Dvalue = 0.0f;
    float xSliderValue  = 0.0f;
    float ySliderValue  = 0.0f;
    float zSliderValue  = 0.0f;
    float xwSliderValue = 0.0f;
    float ywSliderValue = 0.0f;
    float zwSliderValue = 0.0f;
};

#endif
