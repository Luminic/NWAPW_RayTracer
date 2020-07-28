#include "Material.hpp"
#include <algorithm>

MaterialHandle::MaterialHandle() : valid(std::make_shared<const bool>(false)) {}

MaterialHandle::MaterialHandle(int material_index, int* material_ref_count, std::shared_ptr<const bool> valid) :
    material_index(material_index),
    material_ref_count(material_ref_count),
    valid(valid)
{
    if (*valid)
        (*material_ref_count)++;
}

MaterialHandle::MaterialHandle(const MaterialHandle& other) :
    material_index(other.material_index),
    material_ref_count(other.material_ref_count),
    valid(other.valid)
{}

MaterialHandle& MaterialHandle::operator=(const MaterialHandle& other) {
    if (&other == this) 
        return *this;

    // Basically destruct the old MaterialHandle
    if (*valid)
        (*material_ref_count)--;

    this->valid = other.valid;
    this->material_index = other.material_index;
    this->material_ref_count = other.material_ref_count;

    // And construct a new one with the new data
    if (*valid)
        (*material_ref_count)++;
    
    return *this;
};

MaterialHandle::~MaterialHandle() {
    if (*valid)
        (*material_ref_count)--;
}

int MaterialHandle::get_material_index() const {
    if (*valid)
        return material_index;
    return -1;
}

Material::Material(glm::vec4 albedo, glm::vec4 F0, float roughness, float metalness, float AO) : 
    albedo(albedo),
    F0(F0),
    roughness(roughness),
    metalness(metalness),
    AO(AO)
{}

Material::Material(int albedo_ti, int F0_ti, int roughness_ti, int metalness_ti, int AO_ti) :
    albedo_ti(albedo_ti),
    F0_ti(F0_ti),
    roughness_ti(roughness_ti),
    metalness_ti(metalness_ti),
    AO_ti(AO_ti)
{}

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