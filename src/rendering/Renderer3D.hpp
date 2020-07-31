#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_5_Core>
#include <vector>

#include "Shader.hpp"
#include "Camera3D.hpp"
#include "Texture.hpp"
#include "objects/Vertex.hpp"
#include "objects/Scene.hpp"

#include "Renderer3DOptions.hpp"

// Forward declaration because they need to know each other
class Renderer3DOptions;

class Renderer3D : public QObject, protected QOpenGLFunctions_4_5_Core{
    Q_OBJECT;
public:
    Renderer3D(QObject* parent=nullptr);
    virtual ~Renderer3D() {}

    // QOpenGLWidget is needed for some functions so context can be made current
    // Other functions assume the context to already be current
    // Assumes the context is current
    Texture* initialize(int width, int height, QOpenGLWidget* opengl_widget=nullptr);

    // Creates renders of size w,h
    // Warning: The render result will still be the old size if iterative rendering
    // is on
    // Assumes the context is current
    void resize(int width, int height);

    // Render new frame (iterative rendering off)
    // or improve previous frame (iterative rendering on)
    // Assumes the context is current
    Texture* render();

    void set_scene(Scene* scene);

    void set_camera(Camera3D* camera);
    Camera3D* get_camera();

    // "Safe" options for settings classes etc.
    Renderer3DOptions* get_options();

    // Iterative rendering improves the quality of the current render_result instead
    // of rendering a new render_result every frame
    void begin_iterative_rendering();
    void end_iterative_rendering();

    // Doesn't need the context to be current: it sets opengl_widget's context as
    // current instead
    // If opengl_widget is null, returns -1 (no mesh) by default
    MeshIndex get_mesh_index_at(int x, int y);

private:
    // Used pretty much only to set context
    // Can be null; if so, functions that need this will return their defaults
    QOpenGLWidget* opengl_widget;

    Texture environment_map;

    Shader render_shader;
    int work_group_size[3];
    Texture render_result;

    // Note: not a "real" opengl vertex shader; rather, this is a compute
    // shader carrying out the function of a vertex shader
    Shader vertex_shader;
    int vertex_shader_work_group_size[3];
    // This MUST match the Y_SIZE in vertex_shader.glsl
    // See definition there for explanation
    static const int Y_SIZE = 64;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    unsigned int vertex_ssbo;
    int vertex_ssbo_size;

    unsigned int static_vertex_ssbo;
    int static_vertex_ssbo_size;
    unsigned int static_index_ssbo;
    int static_index_ssbo_size;

    unsigned int dynamic_vertex_ssbo;
    int dynamic_vertex_ssbo_size;
    unsigned int dynamic_index_ssbo;
    int dynamic_index_ssbo_size;

    std::vector<unsigned char> opengl_mesh_data;
    unsigned int mesh_ssbo;
    int mesh_ssbo_size;

    unsigned int material_ssbo;
    int material_ssbo_size;

    int width;
    int height;

    // Per-pixel mesh indices (output of render)
    // The data at pixel (u,v) is located at u+v*size_x where size_x is mesh_indices_ssbo_size[0]
    unsigned int mesh_indices_ssbo;
    // Per-pixel indices so size is split into width & height (actual buffer size is width*height)
    int mesh_indices_ssbo_size[2];

    // Iterative rendering
    Texture* iterative_render();
    bool iterative_rendering;
    int nr_iterations_done;
    Texture scene_geometry;
    Texture scene_normals;
    int iterative_rendering_texture_size[2];

    Scene* scene;
    void add_meshes_to_buffer();
    void add_mesh_vertices_to_buffer(const std::vector<AbstractMesh*>& meshes, unsigned int vert_ssbo, int mesh_index_offset=0);
    void add_mesh_indices_to_buffer(const std::vector<AbstractMesh*>& meshes, unsigned int ind_ssbo);

    void add_materials_to_buffer();

    void set_textures();

    Camera3D* camera;
    Renderer3DOptions* options;
};

#endif
