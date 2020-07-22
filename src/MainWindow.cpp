#include "MainWindow.hpp"
#include <QApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Raytracer");
    resize(800, 600);

    Vertex verts[8] = {
        // Floor
        Vertex(glm::vec4(-1.0f,-1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f,-1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f)),

        // Ceiling
        Vertex(glm::vec4(-1.0f, 1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f, 1.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f, 1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f, 1.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f))
    };

    Index inds[12] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4
    };

    StaticMesh<8, 12>* mesh = new StaticMesh<8, 12>(verts, inds, this);
    scene.add_static_mesh((VirtualStaticMesh*)mesh);

    viewport.set_scene(&scene);
    setCentralWidget(&viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);
    elapsedTimer.start();
}

MainWindow::~MainWindow() {}

void MainWindow::main_loop() {
    float dt = elapsedTimer.restart() / 1000.0f;
    viewport.main_loop(dt);
}
