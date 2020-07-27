#include "ModelLoader4D.hpp"
#include <QFile>
#include <QDebug>
#include <glm/glm.hpp>
#include <QRegularExpression>

Node* ModelLoader4D::load_model(const char* file_path) {
    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file:" << file_path;
        return nullptr;
    }

    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> tex_coords;

    std::vector<vert> verts;
    std::vector<Index> indices;
    Index currentIndex = 0;

    bool loading_file = true;
    do {
        QString line(file.readLine().toStdString().c_str());
//        qDebug() << "Line:" << line;

        QRegularExpression regex("[ |\r|\n]"); // \\w+ doesn't work :P
        QStringList tokens = line.split(regex);
//        qDebug() << tokens;

        if (tokens[0] == "v") {
            positions.emplace_back(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat(), tokens[4].toFloat());
        } else if (tokens[0] == "vt") {
            tex_coords.emplace_back(tokens[1].toFloat(), tokens[2].toFloat());
        } else if (tokens[0] == "vn") {
            normals.emplace_back(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat(), tokens[4].toFloat());
        } else if (tokens[0] == "f") {
            for (unsigned char i = 1; i < 5; i++) {
                QStringList vertex_data = tokens[i].split('/');

                vert vertex{vertex_data[0].toUInt() - 1, 0, 0};
                if (vertex_data.length() == 2 || (vertex_data.length() == 3 && vertex_data[1].length()))
                    vertex.tex_coord_index = vertex_data[1].toUInt() - 1;
                if (vertex_data.length() == 3)
                    vertex.normal_index = vertex_data[2].toUInt() - 1;

                bool found = false;
                for (unsigned int j = 0; j < verts.size(); j++) {
                    if (vertex == verts[j]) {
                        found = true;
                        indices.push_back(j);
                        break;
                    }
                }

                if (!found) {
                    verts.push_back(vertex);
                    indices.push_back(currentIndex++);
                }
            }
        }

        if (!line.size()) loading_file = false;
    } while (loading_file);

    qDebug() << verts.size() << indices.size() << positions.size() << normals.size() << tex_coords.size();
    qDebug() << "Verts:";
    for (unsigned int i = 0; i < verts.size(); i++)
        qDebug() << verts[i].position_index << verts[i].normal_index << verts[i].tex_coord_index;

    qDebug() << "Indices:";
    for (unsigned int i = 0; i < indices.size(); i += 4)
        qDebug() << indices[i+0] << indices[i+1] << indices[i+2] << indices[i+3];

    file.close();
    return nullptr;
}
