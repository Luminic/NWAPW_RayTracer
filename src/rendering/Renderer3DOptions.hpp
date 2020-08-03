#ifndef RENDERER_3D_OPTIONS
#define RENDERER_3D_OPTIONS

#include <QObject>
#include "Renderer3D.hpp"

// Forward declaration because they need to know each other
class Renderer3D;

// An interface for safely modifying the many options in Renderer3D
// This class can only be created by the Renderer3D itself and should
// be passed around as a pointer
// See Renderer3D for function documentation
class Renderer3DOptions : public QObject {
    Q_OBJECT;
public:

    void begin_iterative_rendering();
    void end_iterative_rendering();

    bool modify_sunlight(const glm::vec3& direction, const glm::vec3& radiance, float ambient_multiplier=0.0f);
    MeshIndex get_mesh_index_at(int x, int y);

private:
    friend class Renderer3D;
    // Parent should be the same as renderer_3D but its clearer to
    // have to the two separate
    Renderer3DOptions(Renderer3D* renderer_3D, QObject* parent=nullptr);

    Renderer3D* renderer_3D;
};

#endif