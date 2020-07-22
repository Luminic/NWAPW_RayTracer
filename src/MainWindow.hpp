#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>

#include "Viewport.hpp"
#include "rendering/objects/Scene.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private:
    void main_loop();

    QTimer timer;
    QElapsedTimer elapsedTimer;

    Scene scene;
    Viewport viewport;
};

#endif
