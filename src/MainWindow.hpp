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

private:
    void resource_initialization();
    void main_loop();

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

    float prev_rotation_xy = 0.0f;
    float prev_rotation_xz = 0.0f;
    float prev_rotation_xw = 0.0f;
    float prev_rotation_yz = 0.0f;
    float prev_rotation_yw = 0.0f;
    float prev_rotation_zw = 0.0f;
};

#endif
