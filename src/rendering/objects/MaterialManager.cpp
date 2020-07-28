#include "MaterialManager.hpp"

#include <algorithm>

MaterialManager::MaterialManager() {
    valid = std::make_shared<bool>(true);
}

MaterialManager::~MaterialManager() {
    *valid = false;
}

int MaterialManager::add_texture(Texture texture, bool new_tex) {
    if (!new_tex) {
        auto it = std::find(textures.begin(), textures.end(), texture);
        if (it != textures.end()) {
            // Return texture index if it already exists
            return std::distance(textures.begin(), it);
        }
    }
    textures.push_back(texture);
    return textures.size()-1;
}

MaterialHandle MaterialManager::add_material(Material material, bool new_mat) {
    // Try to find repeat materials
    if (!new_mat) {
        auto it = std::find(materials.begin(), materials.end(), material);
        if (it != materials.end()) {
            // Return material index if it already exists
            // return std::distance(materials.begin(), it);
            int i = std::distance(materials.begin(), it);
            return MaterialHandle(i, &material_ref_counts[i], valid);
        }
    }
    // Try to fill in vacated material spots
    auto it = std::find(material_ref_counts.begin(), material_ref_counts.end(), 0);
    if (it != material_ref_counts.end()) {
        int i = std::distance(material_ref_counts.begin(), it);
        materials[i] = material;
        return MaterialHandle(i, &material_ref_counts[i], valid);
    }
    // Add new material to back of vector
    materials.push_back(material);
    int i = materials.size()-1;
    return MaterialHandle(i, &material_ref_counts[i], valid);
}

const std::vector<Texture>& MaterialManager::get_textures() const {
    return textures;
}

const std::vector<Material>& MaterialManager::get_materials() const {
    return materials;
}
