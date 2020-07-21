#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QOpenGLFunctions_4_5_Core>

#include "Shader.hpp"
#include "Texture.hpp"
#include "Camera3D.hpp"
#include "objects/Vertex.hpp"

class Renderer3D : public QObject, protected QOpenGLFunctions_4_5_Core{
    Q_OBJECT;

public:
    Renderer3D(QObject* parent=nullptr);

    Texture* initialize(int width, int height);
    void resize(int width, int height);

    Texture* render();

    void set_camera(Camera3D* camera);
    Camera3D* get_camera();

private:
    Shader render_shader;
    int work_group_size[3];
    Texture render_result;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int vertex_ssbo;
    unsigned int index_ssbo;

    int width;
    int height;

    Camera3D* camera;
};

#endif