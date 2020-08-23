#include "ModelLoader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <QDebug>
#include <glm/glm.hpp>
#include <QRegularExpression>
#include "objects/DynamicMesh.hpp"

#define logError(error) { std::cerr << error; mesh4dIndices.clear(); mesh4dVertices.clear(); return nullptr; }
#define addTetrahedron(p0, p1, p2, p3) { mesh4dIndices.push_back(p0); mesh4dIndices.push_back(p1); mesh4dIndices.push_back(p2); mesh4dIndices.push_back(p3); }

// NOTE: this does NOT handle any malformed files
Node* ModelLoader::load_model(const char* file_path) {
    // Try to open the file
    std::ifstream file = std::ifstream(file_path);
    if (!file.is_open()) {
        qDebug() << "Failed to open file:" << file_path;
        return nullptr;
    } else {
        std::vector<AbstractMesh*> meshes;
        std::vector<Index> mesh4dIndices;
        std::vector<Vertex> mesh4dVertices;

        std::string line;
        PrimitiveType type = PrimitiveType::None;
        size_t lineNumber = 1;

        try {
            while (!file.eof()) {
                // Get the whole line
                std::getline(file, line);

                // Remove comments
                size_t commentIndex = line.find_first_of('#');
                if (commentIndex != std::string::npos)
                    line.erase(commentIndex);

                // If the line has any characters
                if (line.size()) {
                    if (!line.rfind("nm ", 0)) {
                        // Finish previous mesh
                        if (mesh4dIndices.size()) {
                            meshes.push_back(new DynamicMesh(mesh4dVertices, mesh4dIndices));
                            mesh4dVertices.clear();
                            mesh4dIndices.clear();
                        }

                        std::string primitiveType;
                        std::stringstream ss(std::move(line));
                        ss >> primitiveType >> primitiveType;

                        if (!primitiveType.rfind("tetrahedra", 0))
                            type = PrimitiveType::Tetrahedron;
                        else if (!primitiveType.rfind("hexahedra", 0))
                            type = PrimitiveType::Hexahedron;
                        else if (!primitiveType.rfind("octahedra", 0))
                            type = PrimitiveType::Octahedron;
                        else if (!primitiveType.rfind("dodecahedra", 0))
                            type = PrimitiveType::Dodecahedron;
                        else if (!primitiveType.rfind("icosahedra", 0))
                            type = PrimitiveType::Icosahedron;
                        else
                            logError("Unknown primitive type: \"" << primitiveType << "\"\n");
                    } else if (!line.rfind("v ", 0)) {
                        // Convert the line into a std::stringstream to make parsing easier
                        std::stringstream ss(std::move(line));

                        // This is to get rid of the "v "
                        char junk;
                        ss >> junk;

                        // Move the indices into the vector
                        glm::vec4 vertex;
                        ss >> vertex.x;
                        ss >> vertex.y;
                        ss >> vertex.z;
                        ss >> vertex.w;

                        // If there was an error converting from strings to vertices
                        if (!ss) logError("Failed to parse line " << lineNumber << ".\n");

                        // Add the vertex to the mesh's vertices
                        mesh4dVertices.emplace_back(vertex);
                    }
                    // I don't think it makes sense to support these two
                    // Normals will be dynamically calculated on the gpu
                    // 3D Texture coordinates for each cell?
                    //else if (!line.rfind("vt ", 0)) {}
                    //else if (!line.rfind("vn ", 0)) {}
                    else if (!line.rfind("f ", 0)) {
                        if (type == PrimitiveType::None)
                            logError("No primitive type set.\n");

                        // Create a vector with a size equal to the
                        // number of indices in this mesh's faces
                        std::vector<Index> indices(primitiveTypeCountMask & static_cast<size_t>(type));

                        // Convert the line into a std::stringstream to make parsing easier
                        std::stringstream ss(std::move(line));

                        // This is to get rid of the "f "
                        char junk;
                        ss >> junk;

                        // Move the indices into the vector
                        for (auto& index : indices)
                            ss >> index;

                        // If there was an error converting from strings to indices
                        if (!ss) logError("Failed to parse line " << lineNumber << ".\n");

                        // Tetrahedralize the primitive
                        Index count = (primitiveTypeIndexMask & static_cast<size_t>(type)) >> primitiveTypeIndexShift;
                        const Index* inds = primitiveIndices[count];
                        for (Index i = 0; i < primitiveIndexCounts[count]; i += 4)
                            addTetrahedron(indices[inds[i + 0]] - 1, indices[inds[i + 1]] - 1, indices[inds[i + 2]] - 1, indices[inds[i + 3]] - 1);
                    } else {
                        // Keep only the command
                        size_t spaceIndex = line.find_first_of(' ');
                        if (spaceIndex != std::string::npos)
                            line.erase(spaceIndex);

                        logError("Unknown command: \"" << line << "\" on line " << lineNumber << ".\n");
                    }
                }

                lineNumber++;
            }

            file.close();
            meshes.push_back(new DynamicMesh(mesh4dVertices, mesh4dIndices));
            return new Node(meshes, this);
        } catch (...) {
            file.close();
            logError("Failed parsing on line " << lineNumber << ".\n");
        }
    }

    // The file was not found
    return nullptr;
}
