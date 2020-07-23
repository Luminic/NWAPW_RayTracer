#include "Renderer3D.hpp"

#include <QDebug>

uint32_t round_up_to_pow_2(uint32_t x);

Renderer3D::Renderer3D(QObject* parent) : QObject(parent) {
    camera = nullptr;
    scene = nullptr;
}

Texture* Renderer3D::initialize(int width, int height) {
    initializeOpenGLFunctions();
    this->width = width;
    this->height = height;

    // Setup the render shader
    ShaderStage comp_shader{GL_COMPUTE_SHADER, ":/src/rendering/shaders/raytracer.glsl"};

    render_shader.load_shaders(&comp_shader, 1);
    render_shader.validate();

    glGetProgramiv(render_shader.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
    render_result.create(width, height);

    if (camera) {
        camera->update_perspective_matrix(float(width)/height);
    }

    // Set up the Vertex SSBO
    glGenBuffers(1, &vertex_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertex_ssbo);

    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // Set up the Index SSBO
    glGenBuffers(1, &index_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, index_ssbo);

    glBufferData(GL_SHADER_STORAGE_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

    // Clean up
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // Not 100% sure if necessary but just in case
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    return &render_result;
}

void Renderer3D::resize(int width, int height) {
    this->width = width;
    this->height = height;
    render_result.resize(width, height);
    
    if (camera) {
        camera->update_perspective_matrix(float(width)/height);
    }
}

Texture* Renderer3D::render() {
    camera->update_view_matrix();
    CornerRays eye_rays = camera->get_corner_rays();

    if (scene) {
        if (scene->static_meshes_modified(true))
            add_static_meshes_to_buffer();
    }

    glUseProgram(render_shader.get_id());
    render_shader.set_vec3("eye", camera->position);
    render_shader.set_vec3("ray00", eye_rays.r00);
    render_shader.set_vec3("ray10", eye_rays.r10);
    render_shader.set_vec3("ray01", eye_rays.r01);
    render_shader.set_vec3("ray11", eye_rays.r11);

    glBindImageTexture(0, render_result.get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    unsigned int worksize_x = round_up_to_pow_2(width);
    unsigned int worksize_y = round_up_to_pow_2(height);
    glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

    // Clean up & make sure the shader has finished writing to the image
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glUseProgram(0);

    return &render_result;
}

void Renderer3D::add_static_meshes_to_buffer() {
    const std::vector<AbstractMesh*>& static_meshes = scene->get_static_meshes();
    unsigned int nr_vertices = 0;
    unsigned int nr_indices = 0;
    for (AbstractMesh* v_mesh : static_meshes) {
        nr_vertices += v_mesh->size_vertices();
        nr_indices += v_mesh->size_indices();
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nr_vertices*sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    unsigned int current_vertices = 0;
    for (AbstractMesh* v_mesh : static_meshes) {
        unsigned int nr_mesh_vertices = v_mesh->size_vertices();
        if (vertex_is_opengl_compatible) {
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, current_vertices*sizeof(Vertex), nr_mesh_vertices*sizeof(Vertex), v_mesh->get_vertices());
        } else {
            std::vector<unsigned char> vertex_data(VERTEX_STRUCT_SIZE_IN_OPENGL*vertices.size());
            for (unsigned int i=0; i<vertices.size(); i++) {
                vertices[i].as_byte_array(&vertex_data[i*VERTEX_STRUCT_SIZE_IN_OPENGL]);
            }
            glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_data.size(), vertex_data.data(), GL_STATIC_DRAW);
        }
        current_vertices += nr_mesh_vertices;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, nr_indices*sizeof(Index), nullptr, GL_DYNAMIC_DRAW);
    unsigned int current_indices = 0;
    for (AbstractMesh* v_mesh : static_meshes) {
        unsigned int nr_mesh_indices = v_mesh->size_indices();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, current_indices*sizeof(Index), nr_mesh_indices*sizeof(Index), v_mesh->get_indices());
        current_indices += nr_mesh_indices;
    }

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
