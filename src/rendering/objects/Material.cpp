#include "Material.hpp"
#include <algorithm>

Material::Material(glm::vec4 albedo, glm::vec4 F0, float roughness, float metalness, float AO) : 
    albedo(albedo),
    F0(F0),
    roughness(roughness),
    metalness(metalness),
    AO(AO)
{
    albedo_ti = -1;
    F0_ti = -1;
    roughness_ti = -1;
    metalness_ti = -1;
    AO_ti = -1;
}

void Material::as_byte_array(unsigned char byte_array[material_size_in_opengl]) const {
    unsigned char const* tmp = reinterpret_cast<unsigned char const*>(&albedo);
    std::copy(tmp, tmp+16, byte_array);

    tmp = reinterpret_cast<unsigned char const*>(&F0);
    std::copy(tmp, tmp+16, byte_array+16);

    tmp = reinterpret_cast<unsigned char const*>(&roughness);
    std::copy(tmp, tmp+4, byte_array+32);

    tmp = reinterpret_cast<unsigned char const*>(&metalness);
    std::copy(tmp, tmp+4, byte_array+36);

    tmp = reinterpret_cast<unsigned char const*>(&AO);
    std::copy(tmp, tmp+4, byte_array+40);

    tmp = reinterpret_cast<unsigned char const*>(&albedo_ti);
    std::copy(tmp, tmp+4, byte_array+44);

    tmp = reinterpret_cast<unsigned char const*>(&F0_ti);
    std::copy(tmp, tmp+4, byte_array+48);

    tmp = reinterpret_cast<unsigned char const*>(&roughness_ti);
    std::copy(tmp, tmp+4, byte_array+52);

    tmp = reinterpret_cast<unsigned char const*>(&metalness_ti);
    std::copy(tmp, tmp+4, byte_array+56);

    tmp = reinterpret_cast<unsigned char const*>(&AO_ti);
    std::copy(tmp, tmp+4, byte_array+60);
}

bool Material::operator==(const Material& other) {
    constexpr float epsilon = 0.0001f;
    return 
        abs(albedo.r-other.albedo.r) < epsilon &&
        abs(albedo.g-other.albedo.g) < epsilon &&
        abs(albedo.b-other.albedo.b) < epsilon &&

        abs(F0.r-other.F0.r) < epsilon &&
        abs(F0.g-other.F0.g) < epsilon &&
        abs(F0.b-other.F0.b) < epsilon &&

        abs(roughness-other.roughness) < epsilon &&
        abs(metalness-other.metalness) < epsilon &&
        abs(AO-other.AO) < epsilon &&

        albedo_ti == other.albedo_ti &&
        F0_ti == other.F0_ti &&
        roughness_ti == other.roughness_ti &&
        metalness_ti == other.metalness_ti &&
        AO_ti == other.AO_ti;
}