#include "Renderer3D.hpp"

#include <QDebug>

uint32_t round_up_to_pow_2(uint32_t x);

Renderer3D::Renderer3D(QObject* parent) : QObject(parent) {
    camera = nullptr;
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

    Vertex verts[] = {
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

    vertices.insert(vertices.begin(), std::begin(verts), std::end(verts));

    if (vertex_is_opengl_compatible) {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vertices[0])*vertices.size(), vertices.data(), GL_STATIC_DRAW);
    } else {
        std::vector<unsigned char> vertex_data(VERTEX_STRUCT_SIZE_IN_OPENGL*vertices.size());
        for (unsigned int i=0; i<vertices.size(); i++) {
            vertices[i].as_byte_array(&vertex_data[i*VERTEX_STRUCT_SIZE_IN_OPENGL]);
        }
        glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_data.size(), vertex_data.data(), GL_STATIC_DRAW);
    }

    // Set up the Index SSBO
    glGenBuffers(1, &index_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, index_ssbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, index_ssbo);

    unsigned int inds[] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4
    };

    indices.insert(indices.begin(), std::begin(inds), std::end(inds));
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(indices[0])*indices.size(), indices.data(), GL_STATIC_DRAW);

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

void Renderer3D::set_camera(Camera3D* camera) {
    this->camera = camera;
    camera->update_perspective_matrix(float(width)/height);
}

Camera3D* Renderer3D::get_camera() {
    return camera;
}
