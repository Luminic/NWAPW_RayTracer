#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <QObject>
#include <glm/glm.hpp>

class Material : public QObject {
    Q_OBJECT;
public:
    Material(const glm::vec4& specular, const glm::vec4& diffuse, const glm::vec4& ambient, QObject* parent=nullptr);
    virtual ~Material() {}

    inline const glm::vec4& get_specular() const { return specular; }
    inline const glm::vec4& get_diffuse() const { return diffuse; }
    inline const glm::vec4& get_ambient() const { return ambient; }
private:
    glm::vec4 specular, diffuse, ambient;
};

#endif
