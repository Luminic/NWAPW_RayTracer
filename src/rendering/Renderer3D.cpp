#include "Renderer3D.hpp"
#include <QDebug>
#include <glm/gtc/type_ptr.hpp>
#include <string>

uint32_t round_up_to_pow_2(uint32_t x);

Renderer3D::Renderer3D(QObject* parent) : QObject(parent) {
    opengl_context = nullptr;
    surface = nullptr;
    camera = nullptr;
    scene = nullptr;
    options = new Renderer3DOptions(this, this);
}

Texture* Renderer3D::initialize(int width, int height, QOpenGLContext* opengl_context, QSurface* surface) {
    this->opengl_context = opengl_context;
    this->surface = surface;

    initializeOpenGLFunctions();
    this->width = width;
    this->height = height;

    iterative_rendering = false;

    if (camera) {
        camera->update_perspective_matrix(float(width)/height);
    }

    environment_map.load_cube_map("resources/textures/lakeside_4k.hdr", 512);

    // Setup the render shader
    ShaderStage comp_shader{GL_COMPUTE_SHADER, "src/rendering/shaders/raytracer.glsl"};

    render_shader.load_shaders(&comp_shader, 1);
    render_shader.validate();

    glGetProgramiv(render_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
    render_result.create(width, height);
    scene_geometry.create(width, height);
    scene_normals.create(width, height);

    // Setup the vertex shader
    ShaderStage vert_shader{GL_COMPUTE_SHADER, "src/rendering/shaders/vertex_shader.glsl"};

    vertex_shader.load_shaders(&vert_shader, 1);
    vertex_shader.validate();

    glGetProgramiv(vertex_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, vertex_shader_work_group_size);

    // Set up the SSBOs
    glGenBuffers(1, &vertex_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    vertex_ssbo_size = 0;

    glGenBuffers(1, &static_vertex_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_vertex_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, static_vertex_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    static_vertex_ssbo_size = 0;

    glGenBuffers(1, &static_index_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_index_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, static_index_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_STATIC_DRAW);
    static_index_ssbo_size = 0;
    
    glGenBuffers(1, &dynamic_vertex_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_vertex_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, dynamic_vertex_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    dynamic_vertex_ssbo_size = 0;

    glGenBuffers(1, &dynamic_index_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_index_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, dynamic_index_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    dynamic_index_ssbo_size = 0;

    glGenBuffers(1, &mesh_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, mesh_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    mesh_ssbo_size = 0;

    glGenBuffers(1, &material_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, material_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, material_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    material_ssbo_size = 0;

    // Initialized to the size of the render_texture for per-pixel mesh index results form render
    glGenBuffers(1, &mesh_indices_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_indices_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, mesh_indices_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, width*height*sizeof(MeshIndex), nullptr, GL_DYNAMIC_READ);
    mesh_indices_ssbo_size[0] = width;
    mesh_indices_ssbo_size[1] = height;

    // Clean up
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Not 100% sure if necessary but just in case
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return &render_result;
}

void Renderer3D::resize(int width, int height) {
    this->width = width;
    this->height = height;
    if (!iterative_rendering) {
        render_result.resize(width, height);
        scene_geometry.resize(width, height);
        scene_normals.resize(width, height);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_indices_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, width*height*sizeof(MeshIndex), nullptr, GL_DYNAMIC_READ);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        mesh_indices_ssbo_size[0] = width;
        mesh_indices_ssbo_size[1] = height;
    }
    
    if (camera) {
        camera->update_perspective_matrix(float(width)/height);
    }
}

Texture* Renderer3D::render() {
    if (iterative_rendering) {
        return iterative_render();
    }

    camera->update_view_matrix();
    CornerRays eye_rays = camera->get_corner_rays();

    Q_ASSERT_X(scene, "Renderer3D::render", "Scene must be set before rendering");
    add_meshes_to_buffer();
    add_materials_to_buffer();
    glUseProgram(vertex_shader.get_id());
    unsigned int vertex_shader_worksize_x = round_up_to_pow_2(vertex_ssbo_size) / Y_SIZE + 1;
    unsigned int vertex_shader_worksize_y = Y_SIZE;
    glDispatchCompute(vertex_shader_worksize_x, vertex_shader_worksize_y, 1);
    // Make sure the vertex shader has finished writing
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(render_shader.get_id());
    render_shader.use_subroutine(GL_COMPUTE_SHADER, "realtime_trace");
    render_shader.set_vec3("eye", camera->position);
    render_shader.set_vec3("ray00", eye_rays.r00);
    render_shader.set_vec3("ray10", eye_rays.r10);
    render_shader.set_vec3("ray01", eye_rays.r01);
    render_shader.set_vec3("ray11", eye_rays.r11);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environment_map.get_id());

    set_textures();

    glBindImageTexture(0, render_result.get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(1, scene_geometry.get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, scene_normals.get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    unsigned int worksize_x = round_up_to_pow_2(width);
    unsigned int worksize_y = round_up_to_pow_2(height);
    glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

    // Clean up & make sure the shader has finished writing to the image
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glUseProgram(0);

    return &render_result;
}

Texture* Renderer3D::iterative_render() {
    glUseProgram(render_shader.get_id());
    render_shader.use_subroutine(GL_COMPUTE_SHADER, "offline_trace");
    render_shader.set_int("nr_iterations_done", nr_iterations_done);
    
    glBindImageTexture(0, render_result.get_id(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, scene_geometry.get_id(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(2, scene_normals.get_id(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    unsigned int worksize_x = round_up_to_pow_2(width);
    unsigned int worksize_y = round_up_to_pow_2(height);
    glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

    // Clean up & make sure the shader has finished writing to the image
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glUseProgram(0);

    nr_iterations_done++;
    return &render_result;
}

Renderer3DOptions* Renderer3D::get_options() {
    return options;
}

void Renderer3D::begin_iterative_rendering() {
    iterative_rendering = true;
    nr_iterations_done = 1;
    iterative_rendering_texture_size[0] = width;
    iterative_rendering_texture_size[1] = height;
}

void Renderer3D::end_iterative_rendering() {
    iterative_rendering = false;

    // Resizing of these images is disabled when iterative rendering
    // So we have to resize them now
    if (iterative_rendering_texture_size[0] != width || iterative_rendering_texture_size[1] != height) {
        render_result.resize(width, height);
        scene_geometry.resize(width, height);
        scene_normals.resize(width, height);
    }
    // mesh_indices_ssbo_size should be equal to iterative_rendering_texture_size
    // but just in case I'll separate them
    if (mesh_indices_ssbo_size[0] != width || mesh_indices_ssbo_size[1] != height) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_indices_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, width*height*sizeof(MeshIndex), nullptr, GL_DYNAMIC_READ);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        mesh_indices_ssbo_size[0] = width;
        mesh_indices_ssbo_size[1] = height;
    }
}

MeshIndex Renderer3D::get_mesh_index_at(int x, int y) {
    if (x >= mesh_indices_ssbo_size[0] || y >= mesh_indices_ssbo_size[1] || x < 0 || y < 0)
        return -1;
    if (opengl_context && surface) {
        opengl_context->makeCurrent(surface);
        MeshIndex mesh_index;
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_indices_ssbo);
        int offset = x;
        offset += (mesh_indices_ssbo_size[1]-(y+1))*mesh_indices_ssbo_size[0];
        offset *= sizeof(MeshIndex);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(MeshIndex), &mesh_index);
        return mesh_index;
    }
    return -1;
}

void Renderer3D::add_meshes_to_buffer() {
    int nr_static_vertices = scene->get_nr_static_vertices();
    int nr_static_indices = scene->get_nr_static_indices();
    int nr_dynamic_vertices = scene->get_nr_dynamic_vertices();
    int nr_dynamic_indices = scene->get_nr_dynamic_indices();

    bool re_add_static_meshes = scene->static_meshes_modified(true);

    if (nr_static_vertices != static_vertex_ssbo_size) {
        static_vertex_ssbo_size = nr_static_vertices;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_vertex_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, nr_static_vertices*sizeof(Vertex), nullptr, GL_STATIC_DRAW);
        re_add_static_meshes = true;
//        qDebug() << "static_vertex_ssbo_size" << static_vertex_ssbo_size;
    }
    if (nr_static_indices != static_index_ssbo_size) {
        static_index_ssbo_size = nr_static_indices;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, static_index_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, nr_static_indices*sizeof(Index), nullptr, GL_STATIC_DRAW);
        re_add_static_meshes = true;
//        qDebug() << "static_index_ssbo_size" << static_index_ssbo_size;
    }
    if (nr_dynamic_vertices != dynamic_vertex_ssbo_size) {
        dynamic_vertex_ssbo_size = nr_dynamic_vertices;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_vertex_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, nr_dynamic_vertices*sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
//        qDebug() << "dynamic_vertex_ssbo_size" << dynamic_vertex_ssbo_size;
    }
    if (nr_dynamic_indices != dynamic_index_ssbo_size) {
        dynamic_index_ssbo_size = nr_dynamic_indices;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, dynamic_index_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, nr_dynamic_indices*sizeof(Index), nullptr, GL_DYNAMIC_DRAW);
//        qDebug() << "dynamic_index_ssbo_size" << dynamic_index_ssbo_size;
    }
    if(nr_dynamic_vertices+nr_static_vertices != vertex_ssbo_size) {
        vertex_ssbo_size = nr_dynamic_vertices+nr_static_vertices;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_ssbo_size*sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
//        qDebug() << "vertex_ssbo_size" << vertex_ssbo_size;
    }

    const std::vector<AbstractMesh*>& static_meshes = scene->get_static_meshes();
    if (re_add_static_meshes) {
        add_mesh_vertices_to_buffer(static_meshes, static_vertex_ssbo);
        add_mesh_indices_to_buffer(static_meshes, static_index_ssbo);
    }

    const std::vector<AbstractMesh*>& dynamic_meshes = scene->get_dynamic_meshes();
    add_mesh_vertices_to_buffer(dynamic_meshes, dynamic_vertex_ssbo, (int)static_meshes.size());
    add_mesh_indices_to_buffer(dynamic_meshes, dynamic_index_ssbo);

    // Send mesh data to shaders
    // Get mesh data into opengl_mesh_data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_ssbo);
    if (static_meshes.size()+dynamic_meshes.size() != (unsigned int)mesh_ssbo_size) {
        mesh_ssbo_size = (int)(static_meshes.size()+dynamic_meshes.size());

        opengl_mesh_data.resize(mesh_ssbo_size*mesh_size_in_opengl, 0);
        for (auto node : scene->get_root_nodes()) {
            node->add_mesh_data(opengl_mesh_data);
        }
        glBufferData(GL_SHADER_STORAGE_BUFFER, mesh_ssbo_size*mesh_size_in_opengl, opengl_mesh_data.data(), GL_DYNAMIC_DRAW);
    } else {
        for (auto node : scene->get_root_nodes()) {
            node->add_mesh_data(opengl_mesh_data);
        }
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, mesh_ssbo_size*mesh_size_in_opengl, opengl_mesh_data.data());
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer3D::add_mesh_vertices_to_buffer(const std::vector<AbstractMesh*>& meshes, unsigned int vert_ssbo, int mesh_index_offset) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vert_ssbo);
    int vertex_offset = 0;
    int mesh_index = mesh_index_offset;
    for (auto mesh : meshes) {
        mesh->vertex_offset = vertex_offset;
        mesh->set_mesh_index(mesh_index);
        mesh_index++;
        int nr_mesh_vertices = (int)mesh->size_vertices();
        if (vertex_is_opengl_compatible) {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, vertex_offset*sizeof(Vertex), nr_mesh_vertices*sizeof(Vertex), mesh->get_vertices());
        } else {
            std::vector<unsigned char> vertex_data(vertex_struct_size_in_opengl*nr_mesh_vertices);
            const Vertex* mesh_vertices = mesh->get_vertices();
            for (int i=0; i<nr_mesh_vertices; i++) {
                mesh_vertices[i].as_byte_array(&vertex_data[i*vertex_struct_size_in_opengl]);
            }
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, vertex_offset*vertex_struct_size_in_opengl, nr_mesh_vertices*vertex_struct_size_in_opengl, vertex_data.data());
        }
        vertex_offset += nr_mesh_vertices;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer3D::add_mesh_indices_to_buffer(const std::vector<AbstractMesh*>& meshes, unsigned int ind_ssbo) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ind_ssbo);
    int index_offset = 0;
    for (auto mesh : meshes) {
        int nr_mesh_indices = (int)mesh->size_indices();
        const Index* mesh_indices = mesh->get_indices();
        Index* indices = new Index[nr_mesh_indices];
        for (int i=0; i<nr_mesh_indices; i++) {
            indices[i] = mesh_indices[i] + mesh->vertex_offset;
        }
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, index_offset*sizeof(Index), nr_mesh_indices*sizeof(Index), indices);
        delete[] indices;
        index_offset += nr_mesh_indices;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer3D::add_materials_to_buffer() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, material_ssbo);
    MaterialManager& material_manager = scene->get_material_manager();
    const std::vector<Material>& materials = material_manager.get_materials();

    if ((int)materials.size() != material_ssbo_size) {
        material_ssbo_size = (int)materials.size();
        glBufferData(GL_SHADER_STORAGE_BUFFER, material_ssbo_size*sizeof(Material), materials.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, material_ssbo_size*sizeof(Material), materials.data());
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Renderer3D::set_textures() {
    MaterialManager& material_manager = scene->get_material_manager();
    const std::vector<Texture*>& textures = material_manager.get_textures();
    for (unsigned int i=0; i<textures.size(); i++) {
        glActiveTexture(GL_TEXTURE1+i);
        glBindTexture(GL_TEXTURE_2D, textures[i]->get_id());
        std::string name = std::string("textures[") + std::to_string(i) + "]";
        render_shader.set_int(name.c_str(), i+1);
    }
    glActiveTexture(GL_TEXTURE0);
}

void Renderer3D::set_scene(Scene* scene) {
    this->scene = scene;
}

void Renderer3D::set_camera(Camera3D* camera) {
    this->camera = camera;
    camera->update_perspective_matrix(float(width)/height);
}

Camera3D* Renderer3D::get_camera() {
    return camera;
}
