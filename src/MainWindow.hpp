#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP

#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
//#include <QPushButton>

#include "Viewport.hpp"
#include "Settings3D.hpp"
#include "Settings4D.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT;

public:
    MainWindow(QWidget* parent=nullptr);
    ~MainWindow();

private:
    void main_loop();

    QTimer timer;
    QElapsedTimer elapsedTimer;

    Viewport viewport;
    //QPushButton *3DSettings = QPushButton("3D Settings");
};

#endif
