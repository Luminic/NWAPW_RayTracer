#ifndef MATERIAL_MANAGER_HPP
#define MATERIAL_MANAGER_HPP

#include <QObject>
#include <vector>
#include <memory>

#include "../Texture.hpp"
#include "Material.hpp"

class MaterialManager : public QObject {
    Q_OBJECT;

public:
    MaterialManager();

    int add_texture(Texture* texture, bool new_tex=false);
    int add_material(Material material, bool new_mat=false);

    const std::vector<Texture*>& get_textures() const;
    const std::vector<Material>& get_materials() const;

private:
    std::vector<Texture*> textures;
    std::vector<Material> materials;
};

#endif