#ifndef TEXTURES_HPP
#define TEXTURES_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>
#include <vector>

struct TextureOptions {
    // Stores pairs of pname and param to be used with glTexImage
    GLenum texture_type;
    std::vector<std::pair<GLenum, GLenum>> options;

    static TextureOptions default_2D_options() {
        return TextureOptions{
            GL_TEXTURE_2D,
            {
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MIN_FILTER, GL_NEAREST),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER, GL_NEAREST)
            }
        };
    }

    static TextureOptions default_3D_options() {
        return TextureOptions{
            GL_TEXTURE_CUBE_MAP,
            {
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MIN_FILTER, GL_LINEAR),
                std::pair<GLenum, GLenum>(GL_TEXTURE_MAG_FILTER, GL_LINEAR)
            }
        };
    }

    // Sets texture wrapping options for 2D/3D textures
    void set_texture_wrap(GLenum wrap, bool three_d=false) {
        options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_S, wrap));
        options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_T, wrap));

        if (three_d)
            options.push_back(std::pair<GLenum, GLenum>(GL_TEXTURE_WRAP_R, wrap));
    }
};

class Texture : public QObject, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;
public:
    Texture(QObject* parent=nullptr);
    virtual ~Texture();

    void load(const char* path, GLenum internal_format=GL_RGBA32F);
    void load(QImage img, GLenum internal_format=GL_RGBA32F);
    void create(unsigned int width, unsigned int height, GLenum internal_format=GL_RGBA32F, bool is_int_type=false);

    // Load eq rect map and convert to cubemap
    void load_cube_map(const char* equirectangular_path, unsigned int size);

    // Warning: This WILL clear the image
    void resize(unsigned int width, unsigned int height);

    unsigned int get_id();

    bool operator==(const Texture& other);

private:
    GLenum internal_format;
    bool is_int_type = false;
    void set_params(TextureOptions texture_options, unsigned int tex_id=0); // TODO: add sampler options and make public

    unsigned int id;
};

#endif
