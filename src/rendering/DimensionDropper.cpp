#include "DimensionDropper.hpp"
#include <glm/glm.hpp>
#include <QDebug>
#include "objects/DynamicMesh.hpp"
#include <QMatrix3x3>

// glm's built-in operator== is too precise
// to the point where rounding errors make
// a big enough difference to not be equal
//
// https://stackoverflow.com/questions/14322299/c-stdfind-with-a-custom-comparator
struct compare_vec3s : public std::unary_function<glm::vec3, bool> {
    compare_vec3s(const glm::vec3& v1) : v1(v1) {}

    bool operator()(const glm::vec3& v2) {
        return std::abs(v1.x - v2.x) < epsilon &&
               std::abs(v1.y - v2.y) < epsilon &&
               std::abs(v1.z - v2.z) < epsilon;
    }

    glm::vec3 v1;

    static constexpr float epsilon = 0.000001f;
};

Node* DimensionDropper::drop(Node* node4d, float slice) {
    std::vector<AbstractMesh*> meshes3d;

    for (const auto& mesh4d : node4d->meshes) {
        // calculate intersection points
        std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>> intersections;

        // for each tetrahedron in the mesh
        for (size_t i = 0; i < mesh4d->size_indices(); i += 4) {
            // accumulate all intersections for each tetrahedron
            std::vector<std::pair<glm::vec3, glm::vec3>> tetraIntersections;

            // get the current tetrahedron
            constexpr unsigned char pointCount = 4;
            glm::vec4 points[pointCount] {
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 0]].position,
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 1]].position,
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 2]].position,
                mesh4d->get_vertices()[mesh4d->get_indices()[i + 3]].position
            };

            // for each triangle in the tetrahedron
            for (unsigned char j = 0; j < pointCount; ++j) {
                std::vector<glm::vec3> triIntersections;

                constexpr unsigned char triCount = pointCount - 1;
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

                        glm::vec3 intersection((1.0f - t) * a.x + t * b.x, (1.0f - t) * a.y + t * b.y, (1.0f - t) * a.z + t * b.z);
                        triIntersections.push_back(intersection);
                    }
                }

                // if a single valid line intersection was found in the triangle
                if (triIntersections.size() == 2 && !compare_vec3s(triIntersections[0])(triIntersections[1]))
                    tetraIntersections.push_back({triIntersections[0], triIntersections[1]});
            }

            // 3 = trigon (1 triangle)    (tetrahedron, hexahedron,             dodecahedron)
            // 4 = tetragon (2 triangles) (tetrahedron, hexahedron, octahedron, dodecahedron)
            // 5 = pentagon (3 triangles) (                                     dodecahedron)
            // 6 = hexagon (4 triangles ) (             hexahedron, octahedron, dodecahedron)
            // 7 = heptagon (5 triangles) (                                     dodecahedron)
            // 8 = octagon (6 triangles)  (                                     dodecahedron)
            if (tetraIntersections.size())
               intersections.push_back(tetraIntersections);
        }

        // assemble the intersection points into triangles
        std::vector<glm::vec3> mesh3d_vertices;
        std::vector<Index> mesh3d_indices;

        // for all tetrahedron with intersections
        for (const auto& tetraIntersection : intersections) {
            // if one or two triangles were found
            if (tetraIntersection.size() == 3 || tetraIntersection.size() == 4) {
                // temporary pool of indices to add them in order
                std::vector<Index> inds;
                inds.reserve(8);
                Index verts_added = 0;

                // only add the vertex positions if they are unique
                for (const auto& line : tetraIntersection) {
                    auto it_first = std::find_if(mesh3d_vertices.begin(), mesh3d_vertices.end(), compare_vec3s(line.first));
                    if (it_first == mesh3d_vertices.end()) { inds.push_back((Index)mesh3d_vertices.size()); mesh3d_vertices.push_back(line.first); verts_added++; }
                    else inds.push_back((Index)(it_first - mesh3d_vertices.begin()));

                    auto it_second = std::find_if(mesh3d_vertices.begin(), mesh3d_vertices.end(), compare_vec3s(line.second));
                    if (it_second == mesh3d_vertices.end()) { inds.push_back((Index)mesh3d_vertices.size()); mesh3d_vertices.push_back(line.second); verts_added++; }
                    else inds.push_back((Index)(it_second - mesh3d_vertices.begin()));
                }

                // remove duplicate indices... weird, I know.
                std::sort(inds.begin(), inds.end());
                inds.erase(std::unique(inds.begin(), inds.end()), inds.end());

                // at certain angles, for example 90 degrees,
                // triangles may be generated that have 0
                // volume, that is all of their points are
                // the same point, and since duplicates are
                // removed, inds will be left with only one
                // index, and adding any index other than
                // inds[0] will result in a crash.
                // if that happens, remove this intersection
                if (inds.size() < tetraIntersection.size()) {
                    mesh3d_vertices.erase(mesh3d_vertices.end() - verts_added, mesh3d_vertices.end());
                    continue;
                }

                mesh3d_indices.push_back(inds[0]);
                mesh3d_indices.push_back(inds[1]);
                mesh3d_indices.push_back(inds[2]);
                if (tetraIntersection.size() == 4) {
                    mesh3d_indices.push_back(inds[1]);
                    mesh3d_indices.push_back(inds[3]);
                    mesh3d_indices.push_back(inds[2]);
                }
            }
        }

        // calculate normals and separate vertices with different normals
        std::vector<Vertex> vertices;
        vertices.reserve(mesh3d_vertices.size() * 3);
        std::vector<Index> indices;
        indices.reserve(mesh3d_indices.size());

        // for every triangle
        for (unsigned int i = 0; i < mesh3d_indices.size(); i += 3) {
            glm::vec3 p0 = mesh3d_vertices[mesh3d_indices[i+0]];
            glm::vec3 p1 = mesh3d_vertices[mesh3d_indices[i+1]];
            glm::vec3 p2 = mesh3d_vertices[mesh3d_indices[i+2]];

            glm::vec3 line1 = p0 - p1;
            glm::vec3 line2 = p0 - p2;

            glm::vec4 normal = glm::vec4(glm::normalize(glm::cross(line1, line2)), 0.0f);

            vertices.push_back(Vertex(glm::vec4(p0, 0.0f), normal));
            vertices.push_back(Vertex(glm::vec4(p1, 0.0f), normal));
            vertices.push_back(Vertex(glm::vec4(p2, 0.0f), normal));

            indices.push_back(i+0);
            indices.push_back(i+1);
            indices.push_back(i+2);
        }

        if (indices.size()) {
            qDebug() << "Mesh" << meshes3d.size() << "has" << vertices.size() << "vertices.";
            qDebug() << "Mesh" << meshes3d.size() << "has" << indices.size() << "indices.";
            qDebug() << "Mesh" << meshes3d.size() << "successfully dropped.";
            meshes3d.push_back(new DynamicMesh(vertices, indices, this));
        }
    }

    return meshes3d.size() ? new Node(meshes3d, this) : nullptr;
}
