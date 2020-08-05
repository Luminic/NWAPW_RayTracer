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

// x=0, y=1, z=2, w=3, >3=error
static glm::mat4 rotate(float angle, unsigned char first, unsigned char second) {
    glm::mat4 matrix(1.0f);
    matrix[first][first]  = cosf(angle); matrix[first][second]  = -sinf(angle);
    matrix[second][first] = sinf(angle); matrix[second][second] =  cosf(angle);
    return matrix;
}

static void print_matrix(const glm::mat4& matrix) {
    for (unsigned char i = 0; i < 4; i++)
        qDebug() << matrix[i][0] << matrix[i][1] << matrix[i][2] << matrix[i][3];
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), model_rotation(1.0f) {
    setWindowTitle("Ray Tracer");
    ui.setupUi(this);

    QDir dir;
    model_path = dir.absoluteFilePath("resources/models/4D/pentachoron.ob4");

    loader = new ModelLoader(this);
    dropper = new DimensionDropper(this);

    viewport = new Viewport(this);
    connect(viewport, &Viewport::opengl_initialized, this, &MainWindow::resource_initialization);

    viewport->set_scene(&scene);
    setCentralWidget(viewport);

    // Setup 3D settings ui
    modelLabel = findChild<QLabel*>("modelLabel");
    modelLabel->setText(truncate_path(model_path));
    settings3D = new Settings3D(this);

    show();

    connect(&timer, &QTimer::timeout, this, &MainWindow::main_loop);

    timer.start(16);
    elapsedTimer.start();
}

void MainWindow::resource_initialization() {
    qDebug() << "resource initialization";

    // Must convert file paths from QStrings to char*
    QByteArray char_model_path = model_path.toLocal8Bit();
    loaded_model = loader->load_model(char_model_path);
    sliced_node = dropper->drop(loaded_model, position_w);
    update_rotation();
    scene.add_root_node(sliced_node);

    // Load mats into material manager object
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

    // Node* monkey = loader->load_model("resources/models/monkey.obj");
    // for (auto m : monkey->meshes)
    //     m->material_index = metal_material;
    // scene.add_root_node(monkey);

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

    update_rotation();
    update_model_rotation();

    if (viewport->is_mouse_pressed()) {
        // Reset input for next update
        viewport->reset_pressed();

        // Get the selected mesh and its parent node
        AbstractMesh* selected_mesh = scene.get_mesh(viewport->get_selected_mesh_index());
        qDebug() << "Selected Mesh" << selected_mesh;
        selected_node = selected_mesh ? selected_mesh->get_node_parent() : nullptr;
        update_transformation();
    }
}

QString MainWindow::truncate_path(QString path) {
    return path.section('/', -1);
}

void MainWindow::on_iterativeRenderCheckBox_toggled(bool checked) {
    settings3D->toggle_iterative_rendering(checked, viewport->get_renderer_3D_options());
}

void MainWindow::on_fileButton_clicked() {
    // Only 4D models are allowed to be loaded
    QString new_model_path = QFileDialog::getOpenFileName(this, "Load a model", "./resources/models/4D/", ("Model Files (*.ob4)"));
    // If a file was selected
    if (new_model_path.length()) {
        modelLabel->setText(truncate_path(new_model_path));
        model_path = new_model_path;

        // Remove the sliced node from the scene and delete it and its meshes
        scene.remove_root_node(sliced_node);
        for (auto mesh : sliced_node->meshes)
            delete mesh;
        delete sliced_node;

        // Delete the loaded model and load the new model
        delete loaded_model;
        loaded_model = loader->load_model(model_path.toLocal8Bit());

        // Slice the new model and add it to the scene
        sliced_node = dropper->drop(loaded_model, position_w);
        update_rotation();
        scene.add_root_node(sliced_node);
    }
}

void MainWindow::update_transformation() {
    if (selected_node) {
        selected_node->transformation = glm::translate(glm::mat4(1.0f), glm::vec3(position_x, position_y, position_z));
        selected_node->transformation = glm::rotate(selected_node->transformation, glm::radians(rotation_x), glm::vec3(1.0f, 0.0f, 0.0f));
        selected_node->transformation = glm::rotate(selected_node->transformation, glm::radians(rotation_y), glm::vec3(0.0f, 1.0f, 0.0f));
        selected_node->transformation = glm::rotate(selected_node->transformation, glm::radians(rotation_z), glm::vec3(0.0f, 0.0f, 1.0f));
    }
}

void MainWindow::update_model_rotation() {
    model_rotation = rotate(glm::radians(rotation_xw), 0, 3) *
                     rotate(glm::radians(rotation_yw), 1, 3) *
                     rotate(glm::radians(rotation_zw), 2, 3);
}

void MainWindow::update_rotation() {
    // 4D rotations must be applied before the model is sliced
    // Since the loaded model should remain untouched, this means
    // there must be an itermediate step in which the rotation
    // is applied, then the result of that step is sliced

    // For every mesh in the model
    for (size_t i = 0; i < loaded_model->meshes.size(); i++) {
        // Get current loaded mesh
        // Loaded models always consist of only dynamic meshes, this is fine
        DynamicMesh* loaded_mesh = dynamic_cast<DynamicMesh*>(loaded_model->meshes[i]);

        // Get vertices and indices
        std::vector<Vertex>& loaded_mesh_vertices = loaded_mesh->modify_vertices();
        std::vector<Index>& loaded_mesh_indices = loaded_mesh->modify_indices();

        // Copy and rotate vertices
        std::vector<Vertex> rotated_mesh_vertices;
        rotated_mesh_vertices.reserve(loaded_mesh_vertices.size());
        for (const auto& vertex : loaded_mesh_vertices)
            rotated_mesh_vertices.push_back(model_rotation * vertex.position);

        // Copy indices
        std::vector<Index> rotated_mesh_indices;
        rotated_mesh_indices.reserve(loaded_mesh_indices.size());
        for (const auto& index : loaded_mesh_indices)
            rotated_mesh_indices.push_back(index);

        // Create a temporary mesh and node
        DynamicMesh* rotated_mesh = new DynamicMesh(rotated_mesh_vertices, rotated_mesh_indices);
        Node* rotated_model = new Node(std::vector<AbstractMesh*>{rotated_mesh});

        // Slice the newly created mesh
        Node* new_sliced_node = dropper->drop(rotated_model, position_w);

        // Get the old mesh's vertices and indices
        DynamicMesh* old_sliced_mesh = dynamic_cast<DynamicMesh*>(sliced_node->meshes[i]);
        std::vector<Vertex>& vertices = old_sliced_mesh->modify_vertices();
        std::vector<Index>& indices = old_sliced_mesh->modify_indices();

        // Clear the old mesh's vertices and indices
        vertices.clear();
        indices.clear();

        // DimensionDropper always returns a node of dynamic meshs, so this is safe
        // In this case, only one mesh went in, so only one mesh comes out
        // Even if the 3D slice did not overlap with the shape, an empty mesh
        // is added to the returned node
        DynamicMesh* new_sliced_mesh = dynamic_cast<DynamicMesh*>(new_sliced_node->meshes[0]);

        // Get the new vertices and indices
        std::vector<Vertex>& new_sliced_mesh_vertices = new_sliced_mesh->modify_vertices();
        std::vector<Index>& new_sliced_mesh_indices = new_sliced_mesh->modify_indices();

        // Fill the old mesh's vertices and indices with the new ones
        vertices.insert(vertices.begin(), new_sliced_mesh_vertices.begin(), new_sliced_mesh_vertices.end());
        indices.insert(indices.begin(), new_sliced_mesh_indices.begin(), new_sliced_mesh_indices.end());
    }
}
