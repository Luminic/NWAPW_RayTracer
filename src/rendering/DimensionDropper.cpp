#include "DimensionDropper.hpp"
#include <glm/glm.hpp>
#include <QDebug>
#include "objects/DynamicMesh.hpp"
#include <QMatrix3x3>

Node* DimensionDropper::drop(Node* node4d, float slice) {
    std::vector<AbstractMesh*> meshes3d;

    for (const auto& mesh4d : node4d->meshes) {
        std::vector<Vertex> mesh3d_vertices;
        std::vector<Index> mesh3d_indices;

        // accumulate all intersections
        std::vector<std::vector<std::vector<glm::vec3>>> intersections;

        // for each tetrahedron in the mesh
        for (size_t i = 0; i < mesh4d->size_indices(); i += 4) {
            // accumulate all intersections for each tetrahedron
            std::vector<std::vector<glm::vec3>> tetraIntersections;

            // get the current tetrahedron
            constexpr unsigned char pointCount = 4;
            glm::vec4 points[pointCount] {
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 0]].position,
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 1]].position,
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 2]].position,
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 3]].position
            };

            // calculate the intersection points for the tetrahedron

            // for each triangle in the tetrahedron
            constexpr unsigned char triCount = pointCount - 1;
            for (unsigned char j = 0; j < triCount; ++j) {

                std::vector<glm::vec3> triIntersections;

                glm::vec4 triPoints[triCount] {
                    points[0 + (j <= 0)],
                    points[1 + (j <= 1)],
                    points[2 + (j <= 2)]
                };

                // for each line segment in the triangle
                for (unsigned char k = 0; k < triCount; ++k) {
                    glm::vec4 a = triPoints[k];
                    glm::vec4 b = triPoints[(k + 1) % triCount];

                    a.w -= slice;
                    b.w -= slice;

                    // this also removes any intersections that
                    // exactly intersect either end of the line
                    if (a.w * b.w < 0.0f) {
                        // the edge intersects
                        float t = a.w / (a.w - b.w);

                        glm::vec3 intersection = { (1.0f - t) * a.x + t * b.x, (1.0f - t) * a.y + t * b.y, (1.0f - t) * a.z + t * b.z };
                        triIntersections.push_back(intersection);
                    }
                }

                tetraIntersections.push_back(triIntersections);
            }

            intersections.push_back(tetraIntersections);
        }

        unsigned int currentIndex = 0;

        for (unsigned int i = 0; i < intersections.size();) {
            if (intersections[i].size() == 3) {
//                qDebug() << "is 3\n";

                std::vector<unsigned int> triIndices;

                for (unsigned int j = 0; j < intersections[i].size();) {
                    if (intersections[i][j].size() == 2) {
//                        qDebug() << " is 2\n";
                        for (const auto &intersection : intersections[i][j]) {
//                            qDebug() << "  mesh3d.vertices.size() = " << mesh3d_vertices.size() << '\n';
                            bool found = false;

                            // for every vertex in the mesh
                            for (unsigned int k = 0; k < mesh3d_vertices.size(); k++) {
                                // if the vertex exists in the mesh already
                                if (mesh3d_vertices[k] == intersection) {
                                    found = true;

                                    bool found2 = false;
                                    // for every index in the triangle
                                    for (unsigned int l = 0; l < triIndices.size(); l++) {
                                        // if the index exists in the triangle already
                                        if (triIndices[l] == k) {
                                            found2 = true;
                                            break;
                                        }
                                    }
                                    if (!found2)
                                        triIndices.push_back(k);

                                    break;
                                }
                            }

                            // if the vertex isn't already in the mesh
                            if (!found) {
                                mesh3d_vertices.push_back(Vertex(glm::vec4(intersection, 0.0f)));
                                triIndices.push_back(currentIndex++);
                            }
                        }

                        j++;
                    } else {
                        intersections[i].erase(intersections[i].begin() + j);
                    }
                }
                mesh3d_indices.insert(mesh3d_indices.end(), triIndices.begin(), triIndices.end());
                i++;
            } else {
                intersections.erase(intersections.begin() + i);
            }
        }

        // TODO: how to i calculate normals from 4d normals?
        // this is just taking the 3d normals of each generated triangle
        for (unsigned int i = 0; i < mesh3d_indices.size(); i += 3) {
            // glm has built-in conversions from 4D to 3D
            // really though, it's just c++ default behaivor
            glm::vec3 p0 = mesh3d_vertices[mesh3d_indices[i+0]].position;
            glm::vec3 p1 = mesh3d_vertices[mesh3d_indices[i+1]].position;
            glm::vec3 p2 = mesh3d_vertices[mesh3d_indices[i+2]].position;

            glm::vec3 line1 = p0 - p1;
            glm::vec3 line2 = p0 - p2;

            glm::vec3 normal = glm::cross(line1, line2);
        }

        qDebug() << "Vertices:";
        for (const auto& vertex : mesh3d_vertices)
            qDebug() << vertex.position.x << vertex.position.y << vertex.position.z;
        qDebug() << "Indices:";
        for (unsigned int i = 0; i < mesh3d_indices.size(); i += 3)
            qDebug() << mesh3d_indices[i+0] << mesh3d_indices[i+1] << mesh3d_indices[i+2];

        meshes3d.push_back(new DynamicMesh(mesh3d_vertices, mesh3d_indices, this));
    }

    return new Node(meshes3d, this);
}
