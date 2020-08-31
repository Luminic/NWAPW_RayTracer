#ifndef MODEL_LOADER_DATA_HPP
#define MODEL_LOADER_DATA_HPP

#include "objects/Vertex.hpp"

// ------------------------- Platonic Solids ------------------------- //
/* Vertex order does not matter. */
constexpr Index Tetrahedron[]
{
     0,  1,  2,  3
};

/* Think of it as 4-bit binary:
 * 0000, 0001, 0010, 0011 ...
 * Start with each negative coordinate
 * and with x as the LSB and W as the MSB,
 * count upwards in binary:
 * -x,-y,-z,-w
 * +x,-y,-z,-w
 * -x,+y,-z,-w
 * ...
 * +x,-y,+z,+w
 * -x,+y,+z,+w
 * +x,+y,+z,+w
 */
constexpr Index Hexahedron[]
{
     0,  1,  2,  4,
     1,  4,  5,  7,
     2,  4,  6,  7,
     1,  2,  3,  7,
     1,  2,  4,  7
};

/* Left, bottom, back, right, up, front. */
constexpr Index Octahedron[]
{
     0,  1,  2,  5,
     0,  4,  2,  5,
     3,  1,  2,  5,
     3,  4,  2,  5
};

/* Start with the cube vertices.
 * Then add the
 * left ones, right ones, -> (bottom then top),
 * bottom ones, top ones, -> (back then front),
 * back ones, front ones. -> (left then right).
 */
constexpr Index Dodecahedron[]
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

/* Left ones, right ones, -> (back then front),
 * bottom ones, top ones, -> (left then right),
 * back ones, front ones. -> (bottom then top).
 */
constexpr Index Icosahedron[]
{
     6,  7,  9, 11,
     0,  1,  6,  9,
     0,  4,  8,  9,
     2,  5,  8,  9,
     2,  3,  7,  9,
     3,  7, 10, 11,
     2,  3,  5, 10,
     4,  5,  8, 10,
     0,  1,  4, 10,
     1,  6, 10, 11,
     7,  9, 10, 11,
     6,  9, 10, 11,
     1,  6,  9, 10,
     3,  7,  9, 10,
     4,  8,  9, 10,
     5,  8,  9, 10,
     0,  4,  9, 10,
     2,  5,  9, 10,
     0,  1,  9, 10,
     2,  3,  9, 10
};

// ------------------------------ Prisms ----------------------------- //
/* bottom then top counter clockwise */
constexpr Index TriangularPrism[]
{
     1,  3,  4,  5,
     0,  1,  2,  3,
     1,  2,  3,  5
};

/* bottom then top counter clockwise */
constexpr Index PentagonalPrism[]
{
     1,  2,  3,  7,
     3,  7,  8,  9,
     0,  3,  4,  9,
     1,  6,  7,  9,
     1,  3,  7,  9,
     0,  1,  3,  9,
     0,  1,  5,  9,
     1,  5,  6,  9,
     1,  3,  7,  9
};

#endif
