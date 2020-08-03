#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <string>

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <QtUiTools>
#include <QApplication>
#include <QTextStream>
#include <QByteArray>

#include "Viewport.hpp"
#include "rendering/objects/Scene.hpp"
#include "rendering/objects/StaticMesh.hpp"
#include "rendering/objects/DynamicMesh.hpp"
#include "rendering/ModelLoader3D.hpp"
#include "rendering/ModelLoader4D.hpp"
#include "rendering/DimensionDropper.hpp"

#include "Settings3D.hpp"
#include "Settings4D.hpp"
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT;
public:
    MainWindow(QWidget* parent=nullptr);
    virtual ~MainWindow() {}

    float return_slider4D_val();
    bool return_file_changed();
    void set_file_changed(bool);

    QString get_new_model_path() const;

private slots:
    void on_iterativeRenderCheckBox_toggled(bool checked);

    void on_fileButton_clicked();

    void on_slice4DSlider_sliderMoved(int position);

    void on_rotateXSlider_sliderMoved(int position);
    void on_rotateYSlider_sliderMoved(int position);
    void on_rotateZSlider_sliderMoved(int position);
    void on_rotateXWSlider_sliderMoved(int position);
    void on_rotateYWSlider_sliderMoved(int position);
    void on_rotateZWSlider_sliderMoved(int position);

private:
    void resource_initialization();
    void main_loop();

    QString truncate_path(QString path);

    Ui::MainWindow ui;

    QTimer timer;
    QElapsedTimer elapsedTimer;

    Scene scene; // if viewport and loader are pointers, should this?
    Viewport* viewport;
    ModelLoader3D* loader3d;
    ModelLoader4D* loader4d;
    DimensionDropper* dropper;
    QString model_path;

    // TODO: move this to somewhere more suitable
    Node* model4d = nullptr;
    Node* sliced_node = nullptr;
    float previous_slice = 0.0f;

    float prev_rotation_x = 0.0f;
    float prev_rotation_y = 0.0f;
    float prev_rotation_z = 0.0f;
    float prev_rotation_xw = 0.0f;
    float prev_rotation_yw = 0.0f;
    float prev_rotation_zw = 0.0f;

    float xSliderValue = 0.0f;
    float ySliderValue = 0.0f;
    float zSliderValue = 0.0f;
    float xwSliderValue = 0.0f;
    float ywSliderValue = 0.0f;
    float zwSliderValue = 0.0f;
    void update_transformation();

    AbstractMesh* selected_mesh = nullptr;

    Settings3D* settings3D;
    QLabel* modelLabel;

    float slider4Dvalue = 0.0f;
};

#endif
