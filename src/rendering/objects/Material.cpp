#include "Material.hpp"

Material::Material(const glm::vec4& specular, const glm::vec4& diffuse, const glm::vec4& ambient, QObject* parent)
    : QObject(parent), specular(specular), diffuse(diffuse), ambient(ambient)
{}
