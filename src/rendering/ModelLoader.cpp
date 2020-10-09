#include "ModelLoader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <QDebug>
#include <glm/glm.hpp>
#include <QRegularExpression>
#include "objects/DynamicMesh.hpp"

#define AddPrimitive(indices, vertexCount) primitives[#indices] = { indices, sizeof(indices) / sizeof(Index), vertexCount }
#define LogError(error) { std::cerr << error; return nullptr; }

Node* ModelLoader::load_model(const char* file_path) {
    // Try to open the file
    std::ifstream file(file_path);
    if (!file.is_open()) {
        qDebug() << "Failed to open file: " << file_path;
        return nullptr;
    } else {
        std::vector<AbstractMesh*> meshes;
        std::vector<Index> mesh4dIndices;
        std::vector<Vertex> mesh4dVertices;

        std::unordered_map<std::string, Primitive> primitives;
        AddPrimitive(Tetrahedron,       4);
        AddPrimitive(Hexahedron,        8);
        AddPrimitive(Octahedron,        6);
        AddPrimitive(Dodecahedron,     20);
        AddPrimitive(Icosahedron,      12);
        AddPrimitive(TriangularPrism,   6);
        AddPrimitive(PentagonalPrism,  10);
        AddPrimitive(TetragonalPyramid, 5);
        AddPrimitive(PentagonalPyramid, 6);

        const std::string Custom = "Custom";
        primitives[Custom] = Primitive();
        std::string type = "Tetrahedron";
        size_t lineNumber = 1;

        // Stuff to do with custom primitives
        std::string customName;
        Index customTetrahedronCount = 0;
        // This primitive's data is overriden
        // every time the file contains a
        // nameless custom primitive
        Primitive* const customPrimitive = &primitives[Custom];
        Primitive* currentCustomPrimitive = customPrimitive;

        try {
            while (!file.eof()) {
                // Get the whole line
                std::string fileLine;
                std::getline(file, fileLine);

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

                    if (customTetrahedronCount && token0 != "ci")
                        LogError("Incomplete custom type definition on line " << lineNumber << ".\n");

                    if (token0 == "nm") {
                        // If there's any mesh data
                        if (mesh4dIndices.size()) {
                            // Finish previous mesh
                            meshes.push_back(new DynamicMesh(mesh4dVertices, mesh4dIndices));
                            mesh4dVertices.clear();
                            mesh4dIndices.clear();
                        }
                    } else if (token0 == "pt") {
                        line >> type;

                        if (type == "Custom") {
                            Index vertexCount;
                            line >> customTetrahedronCount >> vertexCount;
                            bool failedToReadCounts = !line;

                            std::string oldCustomName = customName;
                            line >> customName;

                            if (!line) {
                                customName = Custom;
                                if (oldCustomName != customName) {
                                    if (failedToReadCounts) LogError("No tetrahedron count or vertex count specified on line " << lineNumber << ".\n");

                                    currentCustomPrimitive = customPrimitive;
                                    currentCustomPrimitive->indices.clear();
                                    currentCustomPrimitive->indices.reserve(customTetrahedronCount * 4);
                                    currentCustomPrimitive->vertexCount = vertexCount;
                                }
                            } else if (primitives.find(customName) == primitives.end()) {
                                primitives[customName] = Primitive();
                                currentCustomPrimitive = &primitives[customName];

                                currentCustomPrimitive->indices.reserve(customTetrahedronCount * 4);
                                currentCustomPrimitive->vertexCount = vertexCount;
                            }
                            else
                                LogError("Redeclared or reference to undeclared primitive \"" << customName << "\" on line " << lineNumber << ".\n");
                        }
                        // If the type was not found
                        else if (primitives.find(type) == primitives.end())
                            LogError("Unknown primitive type: \"" << type << "\" on line " << lineNumber << ".\n");
                    } else if (token0 == "ci") {
                        // This is the "Custom Index" command
                        // This supplies the parser with a custom index

                        if (type != Custom)
                            LogError("Cannot use ci for non-custom primitives on line " << lineNumber << ".\n");

                        Index i0, i1, i2, i3;
                        line >> i0 >> i1 >> i2 >> i3;
                        currentCustomPrimitive->indices.push_back(i0 - 1);
                        currentCustomPrimitive->indices.push_back(i1 - 1);
                        currentCustomPrimitive->indices.push_back(i2 - 1);
                        currentCustomPrimitive->indices.push_back(i3 - 1);

                        customTetrahedronCount--;

                        if ((int)customTetrahedronCount < 0)
                            LogError("Too many custom indices on line " << lineNumber << ".\n");
                    } else if (token0 == "v") {
                        // Move the indices into the vector
                        glm::vec4 vertex;
                        line >> vertex.x;
                        line >> vertex.y;
                        line >> vertex.z;
                        line >> vertex.w;

                        // If there was an error converting from strings to vertices
                        if (!line) LogError("Failed to parse vertex data on line " << lineNumber << ".\n");

                        // Add the vertex to the mesh's vertices
                        mesh4dVertices.emplace_back(vertex);
                    }
                    else if (token0 == "f") {
                        if (type == Custom && !currentCustomPrimitive->indices.size())
                            LogError("No custom primitive data set on line " << lineNumber << ".\n");

                        // Create a vector with a size equal to the
                        // number of indices in this mesh's faces
                        Index indexCount = (type == Custom ? *currentCustomPrimitive : primitives[type]).vertexCount;
                        std::vector<Index> faceIndices(indexCount);

                        // Move the indices into the vector
                        for (auto& index : faceIndices)
                            line >> index;

                        // If there was an error converting from strings to indices
                        if (!line) LogError("Failed to parse indices on line " << lineNumber << ".\n");

                        // Handle the custom primitive case
                        if (type == Custom) {
                            for (Index i = 0; i < currentCustomPrimitive->indices.size(); i++)
                                mesh4dIndices.push_back(faceIndices[currentCustomPrimitive->indices[i]] - 1);
                        } else {
                            // Tetrahedralize the primitive
                            const auto& indices = primitives[type].indices;
                            for (Index i = 0; i < indices.size(); i++)
                                mesh4dIndices.push_back(faceIndices[indices[i]] - 1);
                        }
                    } else
                        LogError("Unknown command: \"" << token0 << "\" on line " << lineNumber << ".\n");
                }

                lineNumber++;
            }

            file.close();
            meshes.push_back(new DynamicMesh(mesh4dVertices, mesh4dIndices));
            return new Node(meshes, this);
        } catch (...) {
            file.close();
            LogError("Failed parsing on line " << lineNumber << ".\n");
        }
    }

    // The file was not found
    return nullptr;
}
