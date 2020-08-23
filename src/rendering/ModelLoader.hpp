#ifndef MODEL_LOADER_4D_HPP
#define MODEL_LOADER_4D_HPP

#include <QObject>
#include "objects/Node.hpp"

constexpr Index primitiveIndexCounts[5]
{
    4, 24, 16, 1, 1
};

constexpr Index tetrahedronIndices[primitiveIndexCounts[0]]
{
    0, 1, 2, 3
};

constexpr Index hexahedronIndices[primitiveIndexCounts[1]]
{
    0, 2, 6, 7,
    7, 4, 6, 0,
    0, 4, 5, 7,
    7, 1, 5, 0,
    7, 2, 3, 0,
    0, 1, 3, 7
};

constexpr Index octahedronIndices[primitiveIndexCounts[2]]
{
    0, 1, 2, 5,
    0, 4, 2, 5,
    3, 1, 2, 5,
    3, 4, 2, 5
};

constexpr Index dodecahedronIndices[primitiveIndexCounts[3]]
{

};

constexpr Index icosahedronIndices[primitiveIndexCounts[4]]
{

};

constexpr const Index* primitiveIndices[5]
{
    tetrahedronIndices,
    hexahedronIndices,
    octahedronIndices,
    dodecahedronIndices,
    icosahedronIndices
};

constexpr unsigned char primitiveTypeCountMask = 0b00011111;
constexpr unsigned char primitiveTypeIndexMask = 0b11100000;
constexpr unsigned char primitiveTypeIndexShift = 5;

enum class PrimitiveType : unsigned char
{
    None         = 0,
    Tetrahedron  = (0 << primitiveTypeIndexShift) | 4,
    Hexahedron   = (1 << primitiveTypeIndexShift) | 8,
    Octahedron   = (2 << primitiveTypeIndexShift) | 6,
    Dodecahedron = (3 << primitiveTypeIndexShift) | 20,
    Icosahedron  = (4 << primitiveTypeIndexShift) | 12
};

class ModelLoader : public QObject {
    Q_OBJECT;
public:
    ModelLoader(QObject* parent=nullptr) : QObject(parent) {}
    virtual ~ModelLoader() {}

    Node* load_model(const char* file_path);
};

#endif
