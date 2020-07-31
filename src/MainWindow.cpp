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
    setWindowTitle("Ray Tracer");
    resize(1280, 640);

    model_path = "resources/models/pentachron.ob4";

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

    // Must convert file paths from QStrings to char*
    QByteArray char_model_path = model_path.toLocal8Bit();
    model4d = loader4d->load_model(char_model_path);
    // cache it here so any model3ds can use it
    // as long as MainWindow owns model4d it's fine
    // but that's less than ideal
    model4d->transformation = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));

    sliced_node = dropper->drop(model4d, 0.0f);
    if (sliced_node) {
        // TODO: these only apply to the first model
        // for some reason and not after the slider
        // is changed...?
        sliced_node->transformation = model4d->transformation;
        for (auto mesh : sliced_node->meshes)
            mesh->material_index = 0;

        scene.add_root_node(sliced_node);
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

    float slice = viewport->return_slider4D_val();
    // this is fine to exactly compare these
    // float values because they will only be
    // exactly equal when the user hasn't
    // moved the slider since the last update
    // which is what I want
    if (previous_slice != slice) {
        previous_slice = slice;

        // range is [-2,2]
        Node* new_sliced_node = dropper->drop(model4d, slice);
        // TODO: make this support more than one mesh
        // the reason I didn't do it right now is because
        // if DimensionDropper slices a mesh and gets no
        // indices, it won't add that mesh to the Node,
        // meaning there needs to be a way to pair each
        // mesh with its new slice.
        // If a slice isn't present, simply clear the
        // vertices and indices for the mesh that
        // belongs to the scene

        // dropper always returns a dynamic mesh, so this is safe
        DynamicMesh* mesh = dynamic_cast<DynamicMesh*>(sliced_node->meshes[0]);

        std::vector<Vertex>& vertices = mesh->modify_vertices();
        std::vector<Index>& indices = mesh->modify_indices();

        vertices.clear();
        indices.clear();

        if (new_sliced_node) {
            DynamicMesh* new_mesh = dynamic_cast<DynamicMesh*>(new_sliced_node->meshes[0]);
            std::vector<Vertex>& new_vertices = new_mesh->modify_vertices();
            std::vector<Index>& new_indices = new_mesh->modify_indices();
            vertices.insert(vertices.begin(), new_vertices.begin(), new_vertices.end());
            indices.insert(indices.begin(), new_indices.begin(), new_indices.end());
        }
    }
}
