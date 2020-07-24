#version 450 core

// Note: this is not a "real" opengl vertex shader
// This carries out many of the same actions but using a compute shader so the data can be stored in an ssbo

struct Vertex {
                    // Base Alignment  // Aligned Offset
    vec4 position;  // 4                  0
                    // 4                  4
                    // 4                  8
                    // 4 (total:16)       12

    vec4 normal;    // 4                  16
                    // 4                  20
                    // 4                  24
                    // 4 (total:16)       28
   
    vec2 tex_coord; // 4                  32 
                    // 4 (total:8)        36

    int mesh_index; // 4                  40

    // (PADDING)    // 4                  44
    // (4 bytes of padding to pad out struct to a multiple of the size of a vec4 because it will be used in an array)

    // Total Size: 48
};

layout (std140, binding=0) buffer VertexBuffer {
    Vertex vertices[];
    //             // Base Alignment  // Aligned Offset
    // vertex[0]      48                 0
    // vertex[1]      48                 48
    // vertex[2]      48                 96
    // ...
    // Maximum of 2,666,666 Vertices (128 MB / 48 B)
};

layout (std140, binding=3) buffer StaticVertexBuffer {
    // Same memory layout as VertexBuffer
    Vertex static_vertices[];
};

layout (std140, binding=4) buffer DynamicVertexBuffer {
    Vertex dynamic_vertices[];
};

// layout (std140, binding=3) buffer MatrixBuffer {
//     // Same memory layout as a c++ mat4 array
//     mat4 transformation_matrix;
// }

#define Y_SIZE 64
layout (local_size_x = 8, local_size_y = 8) in;

void main() {
    // The OGL spec says the minimum possible invocations for x, y, and z is 65535
    // The maximum number of elements in VertexBuffer far exceeds this number
    // so a second axis must be used to augment the size of index

    // The corresponding ogl code calling dispatchcompute MUST know this Y_SIZE value and
    // take it into account when dispatching compute
    uint index = gl_GlobalInvocationID.x*Y_SIZE + gl_GlobalInvocationID.y;
    if (index >= vertices.length() || gl_GlobalInvocationID.y >= Y_SIZE) {
        return;
    }

    if (index < static_vertices.length()) {
        vertices[index] = static_vertices[index];
    } else {
        vertices[index + static_vertices.length()] = dynamic_vertices[index];
    }
}