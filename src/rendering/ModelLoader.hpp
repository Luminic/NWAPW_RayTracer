#ifndef MODEL_LOADER_4D_HPP
#define MODEL_LOADER_4D_HPP

#include <QObject>
#include "objects/Node.hpp"

constexpr Index tetrahedronIndices[]
{
    0, 1, 2, 3
};

constexpr Index hexahedronIndices[]
{
    0, 1, 2, 4,
    1, 4, 5, 7,
    2, 4, 6, 7,
    1, 2, 3, 7,
    1, 2, 4, 7
};

constexpr Index octahedronIndices[]
{
    0, 1, 2, 5,
    0, 4, 2, 5,
    3, 1, 2, 5,
    3, 4, 2, 5
};

// You do not want to know the pain
// that I went through to get this data.
constexpr Index dodecahedronIndices[]
{
     2,  3, 14, 15,
     1,  2,  3, 15,
     1,  2,  3, 17,
     1,  2, 16, 17,
     0,  1,  2, 16,
     0,  1,  2,  9,
     0,  1,  4, 12,
     0,  4,  8,  9,
     0,  1,  4,  9,
     1,  2,  9, 15,
     1,  3, 11, 15,
     7, 11, 15, 19,
     6,  9, 15, 18,
     1,  4,  9, 15,
     4,  6,  9, 15,
     4, 15, 18, 19,
     4,  9, 15, 18,
     1,  4,  5, 12,
     4,  5, 12, 13,
     1, 11, 15, 19,
     1,  4, 15, 19,
     1,  4, 15, 19,
     1,  5, 10, 11,
     1,  5, 11, 19,
     1,  4,  5, 19
};

constexpr Index icosahedronIndices[1]
{

};

constexpr const Index* primitiveIndices[]
{
    tetrahedronIndices,
    hexahedronIndices,
    octahedronIndices,
    dodecahedronIndices,
    icosahedronIndices
};
constexpr Index primitiveIndexCounts[]
{
    sizeof(tetrahedronIndices)  / sizeof(Index),
    sizeof(hexahedronIndices)   / sizeof(Index),
    sizeof(octahedronIndices)   / sizeof(Index),
    sizeof(dodecahedronIndices) / sizeof(Index),
    sizeof(icosahedronIndices)  / sizeof(Index)
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
