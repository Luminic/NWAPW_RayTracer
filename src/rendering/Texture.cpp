#include "Texture.hpp"
#include "Shader.hpp"
#include <QImage>
#include <QFile>
#include <QDebug>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

uint32_t round_up_to_pow_2(uint32_t x) {
    /*
    In C++20 we can use:
        #include <bit>
        std::bit_ceil(x)
    */
    // Current implementation from https://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;
    return x;
}

Texture::Texture(QObject* parent) : QObject(parent) {
    id = 0;
}

Texture::~Texture() {
    if (id)
        glDeleteTextures(1, &id);
}

void Texture::load(const char* path, GLenum internal_format) {
    QImage img = QImage(path).convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
    load(img, internal_format);
}

void Texture::load(QImage img, GLenum internal_format) {
    initializeOpenGLFunctions();
    this->internal_format = internal_format;

    glGenTextures(1, &id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    set_params(TextureOptions::default_2D_options());
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img.width(), img.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
}

void Texture::create(unsigned int width, unsigned int height, GLenum internal_format, bool is_int_type) {
    initializeOpenGLFunctions();
    this->internal_format = internal_format;
    this->is_int_type = is_int_type;

    glGenTextures(1, &id);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);

    set_params(TextureOptions::default_2D_options());
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, is_int_type ? GL_RGBA_INTEGER : GL_RGBA, is_int_type ? GL_INT : GL_UNSIGNED_BYTE, (void*)0);
}

void Texture::load_cube_map(const char* equirectangular_path, unsigned int size) {
    initializeOpenGLFunctions();

    stbi_set_flip_vertically_on_load(true);
    int width, height, nr_channels;
    float* data = stbi_loadf(equirectangular_path, &width, &height, &nr_channels, 0);

    unsigned int equirectangular_map;
    glGenTextures(1, &equirectangular_map);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, equirectangular_map);

    set_params(TextureOptions::default_2D_options(), equirectangular_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGB, GL_FLOAT, data);

    stbi_image_free(data);

    // Convert equirectangular-map into a cubemap
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);
    for (int i=0; i<6; i++) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
    }

    set_params(TextureOptions::default_3D_options());

    Shader eq_to_cubemap;
    ShaderStage eq_to_cubemap_compute{GL_COMPUTE_SHADER, "src/rendering/shaders/equirectangular_to_cube_map.glsl"};

    eq_to_cubemap.load_shaders(&eq_to_cubemap_compute, 1);
    eq_to_cubemap.validate();

    glUseProgram(eq_to_cubemap.get_id());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, equirectangular_map);

    glActiveTexture(GL_TEXTURE1);
    glBindImageTexture(1, id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    unsigned int worksize = round_up_to_pow_2(size);
    int work_group_size[3];
    glGetProgramiv(eq_to_cubemap.get_id(), GL_COMPUTE_WORK_GROUP_SIZE, work_group_size);
    glDispatchCompute(worksize/work_group_size[0], worksize/work_group_size[1], 8/work_group_size[2]);

    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glDeleteTextures(1, &equirectangular_map);

    glActiveTexture(GL_TEXTURE0);
}

void Texture::set_params(TextureOptions texture_options, unsigned int tex_id) {
    if (tex_id == 0) {
        tex_id = id;
    }
    
    glBindTexture(texture_options.texture_type, tex_id);

    for (auto options_pair : texture_options.options) {
        glTextureParameteri(tex_id, options_pair.first, options_pair.second);
    }
}

void Texture::resize(unsigned int width, unsigned int height) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, is_int_type ? GL_RGBA_INTEGER : GL_RGBA, is_int_type ? GL_INT : GL_UNSIGNED_BYTE, (void*)0);
}

unsigned int Texture::get_id() {
    return id;
}

bool Texture::operator==(const Texture& other) {
    return id == other.id;
}