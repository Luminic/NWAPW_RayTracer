#include "MaterialManager.hpp"
#include <QDebug>
#include <algorithm>
#include <glm/glm.hpp>

MaterialManager::MaterialManager() {
    // Material 0 is the default material
    Material default_material(glm::vec4(1.0f));
    materials.push_back(default_material);
}

int MaterialManager::add_texture(Texture* texture, bool new_tex) {
    if (!new_tex) {
        for (unsigned int i=0; i<textures.size(); i++) {
            if (*(textures[i]) == *texture) {
                return i;
            }
        }
    }
    textures.push_back(texture);
    return textures.size()-1;
}

int MaterialManager::add_material(Material material, bool new_mat) {
    // Try to find repeat materials
    if (!new_mat) {
        auto it = std::find(materials.begin(), materials.end(), material);
        if (it != materials.end()) {
            // Return material index if it already exists
            return std::distance(materials.begin(), it);
        }
    }
    // Add new material to back of vector
    materials.push_back(material);
    return materials.size()-1;
}

const std::vector<Texture*>& MaterialManager::get_textures() const {
    return textures;
}

const std::vector<Material>& MaterialManager::get_materials() const {
    return materials;
}
