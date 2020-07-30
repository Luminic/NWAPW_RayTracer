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

static void test(DimensionDropper* dropper, Scene* scene, Node* model4d, float slice, int row) {
    Node* model3d = dropper->drop(model4d, slice);
    if (model3d) {
        model3d->transformation = transform(glm::vec3(20.0f * (slice - 0.5f), 0.0f, (row + 2) * -2.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f));
        scene->add_root_node(model3d);
    }
}

static void print_matrix(const glm::mat4& matrix) {
    for (unsigned char i = 0; i < 4; i++)
        qDebug() << matrix[i][0] << matrix[i][1] << matrix[i][2] << matrix[i][3];
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Ray Tracer");
    resize(1280, 640);

    loader3d = new ModelLoader3D(this);
    loader4d = new ModelLoader4D(this);
    dropper = new DimensionDropper(this);

    viewport = new Viewport(this);
    connect(viewport, &Viewport::opengl_initialized, this, &MainWindow::resource_initialization);

    viewport->set_scene(&scene);
    setCentralWidget(viewport);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);

    timer.start(16);
    elapsedTimer.start();
}

void MainWindow::resource_initialization() {
    qDebug() << "resource initialization";

//    Node* model_root_node = loader3d->load_model("resources/models/dodecahedron.obj");
//    model_root_node->transformation = glm::translate(glm::mat4(0.5f), glm::vec3(0.0f, 0.0f, 0.0f));
//    scene.add_root_node(model_root_node);

    Node* model4d = loader4d->load_model("resources/models/pentachron.ob4");

    glm::mat4 rotation_matrix(1.0f);
    constexpr char axis1 = 0, axis2 = 3, increment = 2;
    for (int j = 0; j < 10; j += increment) {
        float rotation = glm::radians(j * 10.0f);
        rotation_matrix[axis1][axis1] = cosf(rotation); rotation_matrix[axis1][axis2] = -sinf(rotation);
        rotation_matrix[axis2][axis1] = sinf(rotation); rotation_matrix[axis2][axis2] = cosf(rotation);

        // TODO: put this in vertex_shader.glsl around line 97
        // to have everything affected by the camera's
        // 4D rotation/position (which should be 4D settings
        // if we don't end up implementing an editor).
        for (auto mesh : model4d->meshes) {
            std::vector<Vertex>& model4d_vertices = dynamic_cast<DynamicMesh*>(mesh)->modify_vertices();
            for (auto& vertex : model4d_vertices)
                vertex.position = rotation_matrix * vertex.position;
        }

        for (int i = 0; i < 10; i += increment) {
            test(dropper, &scene, model4d, i / 10.0f, j);
        }
    }

    MaterialManager& material_manager = scene.get_material_manager();
    Material mat(glm::vec4(0.0f,1.0f,0.0f,1.0f));
    mat.metalness = 1.0f;
    Texture* diffuse_texture = new Texture(&material_manager);
    diffuse_texture->load("resources/textures/Metal022_2K-JPG/Metal022_2K_Color.jpg");
    mat.albedo_ti = material_manager.add_texture(diffuse_texture);
    Texture* rougness_texture = new Texture(&material_manager);
    rougness_texture->load("resources/textures/Metal022_2K-JPG/Metal022_2K_Roughness.jpg");
    mat.roughness_ti = material_manager.add_texture(rougness_texture);
    Texture* metalness_texture = new Texture(&material_manager);
    metalness_texture->load("resources/textures/Metal022_2K-JPG/Metal022_2K_Metalness.jpg");
    mat.metalness_ti = material_manager.add_texture(metalness_texture);
    int metal_material = material_manager.add_material(mat);

    Vertex verts[8] = {
        // Floor
        Vertex(glm::vec4(-1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),

        // Ceiling
        Vertex(glm::vec4(-1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f, 2.0f, 1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(1.0f, 1.0f)),
        Vertex(glm::vec4(-1.0f, 2.0f, 1.0f, 1.0f), glm::vec4(0.0f,-1.0f, 0.5f, 1.0f), glm::vec2(0.0f, 1.0f))
    };
    Index inds[12] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4
    };
    StaticMesh<8, 12>* mesh = new StaticMesh<8, 12>(verts, inds, this);
    mesh->material_index = metal_material;

    Vertex verts1[4] = {
        Vertex{glm::vec4(-1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec4( 1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec4(-1.0f, 1.0f,-1.0f, 1.0f), glm::vec4(0.0f,0.0f,1.0f,1.0f), glm::vec2(0.0f, 1.0f)}
    };
    Index inds1[6] = {
        0, 1, 2,
        2, 3, 0
    };
    StaticMesh<4, 6>* mesh1 = new StaticMesh<4, 6>(verts1, inds1, this);
    mesh1->material_index = metal_material;
    
    std::vector<Vertex> verts2 {
        // Floor 2
        Vertex(glm::vec4(-1.0f,-2.0f,-1.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f), glm::vec2(0.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-2.0f,-1.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f), glm::vec2(1.0f, 0.0f)),
        Vertex(glm::vec4( 1.0f,-2.0f, 1.0f,1.0f), glm::vec4(0.0f,1.0f,0.0f,1.0f), glm::vec2(1.0f, 1.0f)),
    };
    std::vector<Index> inds2 { 0, 1, 2 };
    DynamicMesh* mesh2 = new DynamicMesh(verts2, inds2, this);
    mesh2->material_index = metal_material;

    std::vector<Vertex> verts3 {
       Vertex(glm::vec4( 1.0f,-2.0f,-1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(0.0f, 0.0f)),
       Vertex(glm::vec4( 1.0f,-1.0f,-1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(1.0f, 0.0f)),
       Vertex(glm::vec4( 1.0f,-1.0f, 1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(1.0f, 1.0f)),
       Vertex(glm::vec4( 1.0f,-2.0f, 1.0f, 1.0f), glm::vec4(-1.0f,0.0f,0.0f,1.0f), glm::vec2(0.0f, 1.0f))
    };
    std::vector<Index> inds3 {
        0, 1, 2,
        2, 3, 0
    };
    DynamicMesh* mesh3 = new DynamicMesh(verts3, inds3, this);
    mesh3->material_index = metal_material;

    scene.add_static_mesh((AbstractMesh*)mesh);
    scene.add_static_mesh((AbstractMesh*)mesh1);
    scene.add_dynamic_mesh((AbstractMesh*)mesh2);
    scene.add_dynamic_mesh((AbstractMesh*)mesh3);
    
}

void MainWindow::main_loop() {
    float dt = elapsedTimer.restart() / 1000.0f;
    viewport->main_loop(dt);
}
