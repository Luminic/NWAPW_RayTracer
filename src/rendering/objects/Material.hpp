#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <QObject>
#include <memory>
#include <glm/glm.hpp>

constexpr int material_size_in_opengl = 64;

struct Material {
                        // Base Alignment  // Aligned Offset
    glm::vec4 albedo;   // 4               // 0
                        // 4               // 4
                        // 4               // 8
                        // 4               // 12

    glm::vec4 F0;       // 4               // 16
                        // 4               // 20
                        // 4               // 24
                        // 4               // 28

    float roughness;    // 4               // 32
    float metalness;    // 4               // 36
    float AO;           // 4               // 40

    // Texture indices

    int albedo_ti;      // 4               // 44
    int F0_ti;          // 4               // 48
    int roughness_ti;   // 4               // 52
    int metalness_ti;   // 4               // 56
    int AO_ti;          // 4               // 60

    // Total Size: 64

    Material(glm::vec4 albedo, glm::vec4 F0=glm::vec4(0.0f), float roughness=0.5f, float metalness=0.0f, float AO=0.0f);
    Material(int albedo_ti, int F0_ti=-1, int roughness_ti=-1, int metalness_ti=-1, int AO_ti=-1);
    Material() = default;

    void as_byte_array(unsigned char byte_array[material_size_in_opengl]) const;

    bool operator==(const Material& other);
};
constexpr bool material_is_opengl_compatible = (sizeof(Material) == material_size_in_opengl) && std::is_standard_layout<Material>::value;

#endif
