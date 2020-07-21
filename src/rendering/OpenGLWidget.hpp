#ifndef OPENGL_WIDGET_HPP
#define OPENGL_WIDGET_HPP

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>

#include <vector>

#include "Renderer3D.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core {
    Q_OBJECT;

public:
    OpenGLWidget(QWidget* parent=nullptr);
    ~OpenGLWidget();

    void main_loop();

    void set_renderer(Renderer3D* renderer);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    unsigned int frame_vbo;
    unsigned int frame_vao;
    Shader frame_shader;

    Renderer3D* renderer;
    Texture* render_result;
};

#endif