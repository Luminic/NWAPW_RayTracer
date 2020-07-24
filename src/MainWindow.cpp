#include "MainWindow.hpp"
#include <QApplication>

static QWidget *loadUiFile(QWidget *parent)
{
    QFile file(":/src/MainWindow.ui");
    file.open(QIODevice::ReadOnly);

    QUiLoader loader;
    return loader.load(&file, parent);
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Raytracer");
    resize(800, 600);

    // TODO
    // assimp doesn't work for me (Bruce) if I don't use absolute paths
    // y'all'll get some non-fatal error like this:
    // Failed to load model from: C:/dev/NWAPW_RayTracer/resources/models/cube.obj
    // Assimp Error:  Unable to open file "C:/dev/NWAPW_RayTracer/resources/models/cube.obj".
    //
    // it can be ignored for now, but we really need to work out how
    // to use relative paths because not even qrc works for assimp
    loader = new ModelLoader3D(this);
    Node* model_node = loader->load_model("C:/dev/NWAPW_RayTracer/resources/models/cube.obj");

    loadUiFile(parent);
    Ui::MainWindow ui;
    ui.setupUi(this);

    // Load viewport into UI
    QWidget *viewportWidget;
    viewportWidget = findChild<QWidget*>("viewportWidget");

    viewport = new Viewport(viewportWidget);

    Vertex verts[8] = {
        // Floor
        Vertex(glm::vec4(-1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f)),

        // Ceiling
        Vertex(glm::vec4(-1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec4( 1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f, 1.0f))
    };

    Index inds[12] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4
    };

    StaticMesh<8, 12>* mesh = new StaticMesh<8, 12>(verts, inds, this);

    Vertex verts1[4] = {
        Vertex{glm::vec4(-1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f,1.0f)},
        Vertex{glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(1.0f,0.0f)},
        Vertex{glm::vec4( 1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(1.0f,0.0f)},
        Vertex{glm::vec4(-1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f,1.0f)}
    };

    Index inds1[6] = {
        8, 9, 10,
        10, 11, 8
    };

    int inds1_size = 5;
    inds1_size++;

    StaticMesh<4, 6>* mesh1 = new StaticMesh<4, 6>(verts1, inds1, this);

    std::vector<Vertex> verts2 {
        // Floor 2
        Vertex(glm::vec4(-1.0f,-2.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-2.0f,-1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-2.0f, 1.0f,1.0f), glm::vec4(0.0f), glm::vec2(1.0f, 0.0f)),
    };

    std::vector<Index> inds2 { 0, 1, 2 };

    DynamicMesh* mesh2 = new DynamicMesh(verts2, inds2, this);

    std::vector<Vertex> verts3 {
       Vertex(glm::vec4( 1.0f,-2.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f,1.0f)),
       Vertex(glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f,1.0f)),
       Vertex(glm::vec4( 1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f,1.0f)),
       Vertex(glm::vec4( 1.0f,-2.0f, 1.0f, 1.0f), glm::vec4(0.0f), glm::vec2(0.0f,1.0f))
    };

    std::vector<Index> inds3 {
        3, 4, 5,
        5, 6, 3
    };

    DynamicMesh* mesh3 = new DynamicMesh(verts3, inds3);

    scene.add_static_mesh((AbstractMesh*)mesh);
    scene.add_static_mesh((AbstractMesh*)mesh1);
    scene.add_dynamic_mesh((AbstractMesh*)mesh2);
    scene.add_dynamic_mesh((AbstractMesh*)mesh3);

    // add the model from way up above
    qDebug() << "before";
    scene.add_node_meshes(model_node);
    qDebug() << "after";

    viewport->set_scene(&scene);
    setCentralWidget(viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);
    elapsedTimer.start();
}

MainWindow::~MainWindow() {}

void MainWindow::main_loop() {
    float dt = elapsedTimer.restart() / 1000.0f;
    viewport->main_loop(dt);
}
