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
        qDebug() << "Failed to open file: " << file_path;
        return nullptr;
    } else {
        std::vector<AbstractMesh*> meshes;
        std::vector<Index> mesh4dIndices;
        std::vector<Vertex> mesh4dVertices;

        PrimitiveType type = PrimitiveType::None;
        size_t lineNumber = 1;

        // Stuff to do with custom primitives
        std::vector<Index> customIndices;
        Index customIndexCount;

        try {
            while (!file.eof()) {
                // Get the whole line
                std::string fileLine;
                std::getline(file, fileLine);
                // TODO: think about having a "global-inside-the-while-loop"
                // std::stringstream and change line to be token0.

                // Remove comments
                size_t commentIndex = fileLine.find_first_of('#');
                if (commentIndex != std::string::npos)
                    fileLine.erase(commentIndex);

                // If the line has any characters
                if (fileLine.size()) {
                    // Convert the line to a std::stringstream
                    std::stringstream line(std::move(fileLine));

                    // Get the first token
                    std::string token0;
                    line >> token0;

                    if (token0 == "nm") {
                        // If there's any mesh data
                        if (mesh4dIndices.size()) {
                            // Finish previous mesh
                            meshes.push_back(new DynamicMesh(mesh4dVertices, mesh4dIndices));
                            mesh4dVertices.clear();
                            mesh4dIndices.clear();
                        }
                    } else if (token0 == "pt") {
                        // Get the primitive tyoe
                        std::string primitiveType;
                        line >> primitiveType;

                        // Check which one it is
                        if (primitiveType == "tetrahedra")
                            type = PrimitiveType::Tetrahedron;
                        else if (primitiveType == "hexahedra")
                            type = PrimitiveType::Hexahedron;
                        else if (primitiveType == "octahedra")
                            type = PrimitiveType::Octahedron;
                        else if (primitiveType == "dodecahedra")
                            type = PrimitiveType::Dodecahedron;
                        else if (primitiveType == "icosahedra")
                            type = PrimitiveType::Icosahedron;
                        else if (primitiveType == "custom") {
                            type = PrimitiveType::Custom;
                            customIndices.clear();
                        } else
                            logError("Unknown primitive type: \"" << primitiveType << "\".\n");
                    } else if (token0 == "cc") {
                        // This is the "Custom Count" command
                        // It gives the parser how many indices there are in this custom type

                        if (type != PrimitiveType::Custom)
                            logError("Cannot use ci for non-custom primitives.\n");

                        Index customTetrahedronCount;
                        line >> customTetrahedronCount;
                        line >> customIndexCount;
                        customIndices.reserve(customTetrahedronCount * 4);
                    } else if (token0 == "ci") {
                        // This is the "Custom Index" command
                        // This supplies the parser with a custom index

                        if (type != PrimitiveType::Custom)
                            logError("Cannot use ci for non-custom primitives.\n");

                        Index i0, i1, i2, i3;
                        line >> i0 >> i1 >> i2 >> i3;
                        customIndices.push_back(i0 - 1);
                        customIndices.push_back(i1 - 1);
                        customIndices.push_back(i2 - 1);
                        customIndices.push_back(i3 - 1);
                    } else if (token0 == "v") {
                        // Move the indices into the vector
                        glm::vec4 vertex;
                        line >> vertex.x;
                        line >> vertex.y;
                        line >> vertex.z;
                        line >> vertex.w;

                        // If there was an error converting from strings to vertices
                        if (!line) logError("Failed to parse line " << lineNumber << ".\n");

                        // Add the vertex to the mesh's vertices
                        mesh4dVertices.emplace_back(vertex);
                    }
                    // I don't think it makes sense to support these two
                    // Normals will be dynamically calculated on the gpu
                    // 3D Texture coordinates for each cell?
                    //else if (token0 == "vt") {}
                    //else if (token0 == "vn") {}
                    else if (token0 == "f") {
                        if (type == PrimitiveType::None)
                            logError("No primitive type set.\n");
                        if (type == PrimitiveType::Custom && !customIndices.size())
                            logError("No custom primitive data set.\n");

                        // Create a vector with a size equal to the
                        // number of indices in this mesh's faces
                        Index indexCount = type == PrimitiveType::Custom ? customIndexCount : primitiveTypeCountMask & static_cast<PrimitiveTypeType>(type);
                        std::vector<Index> indices(indexCount);

                        // Move the indices into the vector
                        for (auto& index : indices)
                            line >> index;

                        // If there was an error converting from strings to indices
                        if (!line) logError("Failed to parse line " << lineNumber << ".\n");

                        // Handle the custom primitive case
                        if (type == PrimitiveType::Custom) {
                            for (Index i = 0; i < customIndices.size(); i++)
                                mesh4dIndices.push_back(indices[customIndices[i]] - 1);
                        } else {
                            // Tetrahedralize the primitive
                            Index count = (primitiveTypeIndexMask & static_cast<PrimitiveTypeType>(type)) >> primitiveTypeIndexShift;
                            const Index* inds = primitiveIndices[count];
                            for (Index i = 0; i < primitiveIndexCounts[count]; i++)
                                mesh4dIndices.push_back(indices[inds[i]] - 1);
                        }
                    } else
                        logError("Unknown command: \"" << token0 << "\" on line " << lineNumber << ".\n");
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
