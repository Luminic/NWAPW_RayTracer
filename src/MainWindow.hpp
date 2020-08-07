#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

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
#include "rendering/ModelLoader.hpp"
#include "rendering/DimensionDropper.hpp"

#include "Settings3D.hpp"
#include "ui_MainWindow.h"

class MainWindow : public QMainWindow {
    Q_OBJECT;
public:
    MainWindow(QWidget* parent=nullptr);
    virtual ~MainWindow() {}
private slots:
    void on_iterativeRenderCheckBox_toggled(bool checked);
    void on_fileButton_clicked();

    inline void on_rotateXSlider_sliderMoved(int position)  { rotation_x  = position / 10.0f; update_transformation(); }
    inline void on_rotateYSlider_sliderMoved(int position)  { rotation_y  = position / 10.0f; update_transformation(); }
    inline void on_rotateZSlider_sliderMoved(int position)  { rotation_z  = position / 10.0f; update_transformation(); }
    inline void on_rotateXWSlider_sliderMoved(int position) { rotation_xw = position / 10.0f; update_rotation(); }
    inline void on_rotateYWSlider_sliderMoved(int position) { rotation_yw = position / 10.0f; update_rotation(); }
    inline void on_rotateZWSlider_sliderMoved(int position) { rotation_zw = position / 10.0f; update_rotation(); }

    // in: (int)[-10000,10000], out: (float)[-2,2]
    inline void on_slice4DSlider_valueChanged(int position) { position_w = position / 5000.0f; }

    inline void on_moveXInput_valueChanged(double value) { position_x = (float)value; update_transformation(); }
    inline void on_moveYInput_valueChanged(double value) { position_y = (float)value; update_transformation(); }
    inline void on_moveZInput_valueChanged(double value) { position_z = (float)value; update_transformation(); }

private:
    /**
     * @brief resource_initialization Inits models and materials for rendering.
     */
    void resource_initialization();
    void main_loop();

    Ui::MainWindow ui;

    QTimer timer;
    QElapsedTimer elapsedTimer;

    Scene scene; // if viewport, loader, and dropper are all pointers, should this?
    Viewport* viewport;
    ModelLoader* loader;
    DimensionDropper* dropper;
    QString model_path;

    // TODO: move this to somewhere more suitable
    Node* loaded_model = nullptr;
    glm::mat4 model_rotation;
    bool fourD = false;
    Node* sliced_node = nullptr;

    float rotation_x = 0.0f;
    float rotation_y = 0.0f;
    float rotation_z = 0.0f;
    float rotation_xw = 0.0f;
    float rotation_yw = 0.0f;
    float rotation_zw = 0.0f;

    float position_x = 0.0f;
    float position_y = 0.0f;
    float position_z = 0.0f;
    float position_w = 0.0f;

    void update_transformation();
    void update_model_rotation();
    void update_rotation();

    Node* selected_node = nullptr;

    Settings3D* settings3D;
    QLabel* modelLabel;
    QString truncate_path(QString path);

    float time=0.0f;
    float total_time=0.0f;
};

#endif
