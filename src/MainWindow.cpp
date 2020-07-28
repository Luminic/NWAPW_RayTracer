#include "MainWindow.hpp"
#include <glm/gtc/matrix_transform.hpp>

static QWidget* loadUiFile(QWidget* parent, QString path) {
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    QUiLoader loader;
    return loader.load(&file, parent);
}

static glm::mat4 transform(const glm::vec3& position, float rotation, const glm::vec3& rotation_axis, const glm::vec3& scalar) {
    return glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), scalar), rotation, rotation_axis), position);
}

static void print_matrix(const glm::mat4& matrix) {
    for (unsigned char i = 0; i < 4; i++)
        qDebug() << matrix[i][0] << matrix[i][1] << matrix[i][2] << matrix[i][3];
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Ray tracer");
    resize(800, 600);

    // load the model
    loader3d = new ModelLoader3D(this);
    loader4d = new ModelLoader4D(this);
    dropper = new DimensionDropper(this);

    Node* model_root_node = loader3d->load_model("resources/models/dodecahedron.obj");
    model_root_node->transformation = glm::translate(glm::mat4(0.5f), glm::vec3(3.0f, 0.0f, 0.0f));
    scene.add_root_node(model_root_node);

    Node* model4d = loader4d->load_model("resources/models/pentachron_4D.obj");

    Node* model3d = dropper->drop(model4d, 0.0f);
    if (model3d) {
        model3d->transformation = transform(glm::vec3(3.0f, 0.0f, 0.0f), glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f));
        scene.add_root_node(model3d);
    }

    /*
    loadUiFile(parent, "src/MainWindow.ui");
    Ui::MainWindow ui;
    ui.setupUi(this);

    // Load viewport into UI
    QWidget *viewportWidget;
    viewportWidget = findChild<QWidget*>("viewportWidget");
    */
    viewport = new Viewport(this);

    Vertex verts[8] = {
        // Floor
        Vertex(glm::vec4(-1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),

        // Ceiling
        Vertex(glm::vec4(-1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec4( 1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec4( 1.0f, 2.0f, 1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(0.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f, 2.0f, 1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(0.0f, 1.0f))
    };

    Index inds[12] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4
    };

    StaticMesh<8, 12>* mesh = new StaticMesh<8, 12>(verts, inds, this);

    Vertex verts1[4] = {
        Vertex{glm::vec4(-1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(0.0f,1.0f)},
        Vertex{glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(1.0f,0.0f)},
        Vertex{glm::vec4( 1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(1.0f,0.0f)},
        Vertex{glm::vec4(-1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(0.0f,1.0f)}
    };

    Index inds1[6] = {
        0, 1, 2,
        2, 3, 0
    };

    int inds1_size = 5;
    inds1_size++;

    StaticMesh<4, 6>* mesh1 = new StaticMesh<4, 6>(verts1, inds1, this);

    std::vector<Vertex> verts2 {
        // Floor 2
        Vertex(glm::vec4(-1.0f,-2.0f,-1.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-2.0f,-1.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-2.0f, 1.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f), glm::vec2(1.0f, 0.0f)),
    };

    std::vector<Index> inds2 { 0, 1, 2 };

    DynamicMesh* mesh2 = new DynamicMesh(verts2, inds2, this);

    std::vector<Vertex> verts3 {
       Vertex(glm::vec4( 1.0f,-2.0f,-1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(0.0f,1.0f)),
       Vertex(glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(0.0f,1.0f)),
       Vertex(glm::vec4( 1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(0.0f,1.0f)),
       Vertex(glm::vec4( 1.0f,-2.0f, 1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(0.0f,1.0f))
    };

    std::vector<Index> inds3 {
        0, 1, 2,
        2, 3, 0
    };

    DynamicMesh* mesh3 = new DynamicMesh(verts3, inds3);

    scene.add_static_mesh((AbstractMesh*)mesh);
    scene.add_static_mesh((AbstractMesh*)mesh1);
    scene.add_dynamic_mesh((AbstractMesh*)mesh2);
    scene.add_dynamic_mesh((AbstractMesh*)mesh3);

    viewport->set_scene(&scene);
    setCentralWidget(viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);
    timer.start(16);
    elapsedTimer.start();
}

void MainWindow::main_loop() {
    float dt = elapsedTimer.restart() / 1000.0f;
    viewport->main_loop(dt);
}
