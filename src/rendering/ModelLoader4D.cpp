#include "ModelLoader4D.hpp"
#include <QFile>
#include <QDebug>
#include <glm/glm.hpp>

Node* ModelLoader4D::load_model(const char* file_path) {
    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file:" << file_path;
        return nullptr;
    }

    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> tex_coords;

    bool loading_file = true;
    do {
        QByteArray line = file.readLine();
        qDebug() << "Line:" << line;
        if (!line.size()) {
            loading_file = false;
        }
    } while (loading_file);

    file.close();
    return nullptr;
}
