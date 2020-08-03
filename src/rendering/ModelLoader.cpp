#include "ModelLoader.hpp"
#include <QFile>
#include <QDebug>
#include <glm/glm.hpp>
#include <QRegularExpression>
#include "objects/DynamicMesh.hpp"

bool operator==(const vert& vert1, const vert& vert2) {
    return vert1.position_index  == vert2.position_index &&
           vert1.normal_index    == vert2.normal_index   &&
           vert1.tex_coord_index == vert2.tex_coord_index;
}

// NOTE: this does NOT handle any malformed files
Node* ModelLoader::load_model(const char* file_path) {
    QFile file(file_path);

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open file:" << file_path;
        return nullptr;
    }

    // all of the meshes in the file
    std::vector<AbstractMesh*> meshes;

    // the actual data
    std::vector<glm::vec4> positions;
    std::vector<glm::vec4> normals;
    std::vector<glm::vec2> tex_coords;

    // mock vertices that store the indices into "the actual data"
    std::vector<vert> verts;

    std::vector<Index> indices;
    Index face_indices = 0;

    bool loading_file = true;
    do {
        // get the line and tokenize it
        QString line(file.readLine().toStdString().c_str());
        QRegularExpression regex("[ |\r|\n]"); // \\w+ doesn't work :P
        QStringList tokens = line.split(regex);

        if (tokens[0] == "v") {
            positions.emplace_back(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat(), tokens[4].toFloat());
        } else if (tokens[0] == "vt") {
            tex_coords.emplace_back(tokens[1].toFloat(), tokens[2].toFloat());
        } else if (tokens[0] == "vn") {
            normals.emplace_back(tokens[1].toFloat(), tokens[2].toFloat(), tokens[3].toFloat(), tokens[4].toFloat());
        } else if (tokens[0] == "f") {
            for (unsigned char i = 1; i < face_indices; i++) {
                QStringList vertex_data = tokens[i].split('/');

                vert vertex{vertex_data[0].toUInt() - 1, 0, 0};
                if (vertex_data.length() == 2 || (vertex_data.length() == 3 && vertex_data[1].length()))
                    vertex.tex_coord_index = vertex_data[1].toUInt() - 1;
                if (vertex_data.length() == 3)
                    vertex.normal_index = vertex_data[2].toUInt() - 1;

                auto it = std::find(verts.begin(), verts.end(), vertex);
                if (it != verts.end()) indices.push_back((Index)(it - verts.begin()));
                else { indices.push_back((Index)verts.size()); verts.push_back(vertex); }
            }
        } // new mesh
        else if (tokens[0] == "nm") {
            store_mesh(verts, indices, positions, normals, tex_coords, meshes);
            verts.clear();
            indices.clear();
            positions.clear();
            normals.clear();
            tex_coords.clear();
            face_indices = tokens[1].toUInt() + 1;
        }

        if (!line.size()) loading_file = false;
    } while (loading_file);
    file.close();

    // get the last mesh in the file
    store_mesh(verts, indices, positions, normals, tex_coords, meshes);
    return new Node(meshes, this);
}

void ModelLoader::store_mesh(const std::vector<vert>& verts,
                const std::vector<Index>& indices,
                const std::vector<glm::vec4>& positions,
                const std::vector<glm::vec4>& normals,
                const std::vector<glm::vec2>& tex_coords,
                std::vector<AbstractMesh*>& meshes) {
    if (!indices.size())
        return;

    std::vector<Vertex> vertices;
    vertices.reserve(verts.size());
    for (const auto& vert : verts) {
        if (normals.size() && tex_coords.size())
            vertices.emplace_back(positions[vert.position_index], normals[vert.normal_index], tex_coords[vert.tex_coord_index]);
        else if (normals.size())
            vertices.emplace_back(positions[vert.position_index], normals[vert.normal_index]);
        else if (tex_coords.size())
            vertices.emplace_back(positions[vert.position_index], glm::vec4(0.0f), tex_coords[vert.tex_coord_index]);
        else
            vertices.emplace_back(positions[vert.position_index]);
    }

    meshes.push_back(new DynamicMesh(vertices, indices, this));
}
