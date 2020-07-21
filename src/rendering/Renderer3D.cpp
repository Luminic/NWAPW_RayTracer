#include "Renderer3D.hpp"

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

    return &render_result;
}

void Renderer3D::resize(int width, int height) {
    this->width = width;
    this->height = height;
    render_result.resize(width, height);
    
    // if (camera) {
    // 
    // }
}

Texture* Renderer3D::render() {
    glUseProgram(render_shader.get_id());
    glBindImageTexture(0, render_result.get_id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    unsigned int worksize_x = round_up_to_pow_2(width);
    unsigned int worksize_y = round_up_to_pow_2(height);
    glDispatchCompute(worksize_x/work_group_size[0], worksize_y/work_group_size[1], 1);

    // Clean up & make sure the shader has finished writing to the image
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    return &render_result;
}

void Renderer3D::set_camera(Camera3D* camera) {
    this->camera = camera;
}

Camera3D* Renderer3D::get_camera() {
    return camera;
}
