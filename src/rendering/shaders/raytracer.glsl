#version 450 core

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
#define DEFUALT_VERTEX Vertex(vec4(0.0f,0.0f,0.0f,-1.0f), vec4(0.0f), vec2(0.0f), 0)

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

layout (std430, binding=1) buffer StaticIndexBuffer {
    // Memory layout should exactly match that of a C++ int array
    int static_indices[];
};

layout (std430, binding=2) buffer DynamicIndexBuffer {
    // Same as StaticIndexBuffer
    // Indices correspond to vertices[dynamic_indices[i] + dynamic_indices_offset]
    int dynamic_indices[];
};

// The number of static vertices
uniform int dynamic_indices_offset;

uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;


#define EPSILON 0.000001f
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f

float ray_plane_int(vec3 ray_origin, vec3 ray_dir, vec3 plane_point, vec3 plane_normal) {
    /*
    A ray is described as ray_orign + t * ray_dir
    This function returns t if the ray intersects the plane. Negative output means no intersection
    
    Ray equation: <x,y,z> = ray_orign + t * ray_dir
    Plane equation: dot(plane_normal, <x,y,z>) + D = 0
    We can calculate D = -dot(plane_normal, plane_point)

    dot(plane_normal, ray_orign + t * ray_dir) + D = 0
    dot(plane_normal, ray_orign) + t * dot(plane_normal, ray_dir) + D = 0
    t = - (D + dot(plane_normal, ray_orign)) / dot(plane_normal, ray_dir)
    */
    float denom = dot(plane_normal, ray_dir);

    if (abs(denom) <= EPSILON) {
        // The ray is parallel to the plane
        return -1;
    }

    float D = -dot(plane_normal, plane_point);
    float numer = -(dot(plane_normal, ray_origin) + D);

    return numer/denom;
}

vec4 barycentric_coordinates(vec3 point, vec3 tri0, vec3 tri1, vec3 tri2) {
    /*
    Returns the barycentric coordinates of point in the triangle tri0-2
    The w value is 1 if the point is inside of the triangle and -1 otherwise
    */
    float double_area_tri = length(cross(tri1-tri0, tri2-tri0));

    float area0 = length(cross(tri1-point, tri2-point)) / double_area_tri;
    float area1 = length(cross(tri0-point, tri2-point)) / double_area_tri;
    float area2 = length(cross(tri0-point, tri1-point)) / double_area_tri;

    if (area0+area1+area2-1 <= EPSILON) {
        // If the combined area of the 3 mini triangles equals the area of the triangle
        // the point is inside of the triangle
        return vec4(area0, area1, area2, 1);
    } else {
        // Otherwise, the point is outside of the triangle
        // Still return the barycentric coordinates because they might still be useful
        return vec4(area0, area1, area2, -1);
    }
}

void triangle_intersection(Vertex v0, Vertex v1, Vertex v2, vec3 ray_origin, vec3 ray_dir, inout float depth, inout Vertex vert) {
    vec3 normal = cross(vec3(v1.position-v0.position), vec3(v2.position-v0.position));
    float rpi = ray_plane_int(ray_origin, ray_dir, v0.position.xyz, normalize(normal));

    // If the ray intersects the triangle
    float dist = rpi*length(ray_dir);
    if (dist >= NEAR_PLANE && dist <= depth) {
        vec3 intersection_point = ray_origin + rpi*ray_dir;
        vec4 bc = barycentric_coordinates(intersection_point, v0.position.xyz, v1.position.xyz, v2.position.xyz);
        // If the point is inside of the triangle
        if (bc.w > 0.0f) {
            depth = dist;
            vert.position = vec4(intersection_point, 1.0f);
            vert.normal = bc.x*v0.normal + bc.y*v1.normal + bc.z*v2.normal;
            vert.tex_coord = bc.x*v0.tex_coord + bc.y*v1.tex_coord + bc.z*v2.tex_coord;
            vert.mesh_index = v0.mesh_index;
        }
    }
}

Vertex get_vertex_data(vec3 ray_origin, vec3 ray_dir) {
    /*
    Returns an interpolated vertex from the intersection between the ray and the
    nearest triangle it collides with

    If there is no triangle, the w component of position will be -1.0f
    otherwise the w component will be 1.0f
    */
    float depth = FAR_PLANE;
    Vertex vert = DEFUALT_VERTEX;
    for (int i=0; i<static_indices.length()/3; i++) {
        Vertex v0 = static_vertices[static_indices[i*3]];
        Vertex v1 = static_vertices[static_indices[i*3+1]];
        Vertex v2 = static_vertices[static_indices[i*3+2]];

        triangle_intersection(v0, v1, v2, ray_origin, ray_dir, depth, vert);
    }
    for (int i=0; i<dynamic_indices.length()/3; i++) {
        Vertex v0 = dynamic_vertices[dynamic_indices[i*3]   ];
        Vertex v1 = dynamic_vertices[dynamic_indices[i*3+1] ];
        Vertex v2 = dynamic_vertices[dynamic_indices[i*3+2] ];

        triangle_intersection(v0, v1, v2, ray_origin, ray_dir, depth, vert);
    }
    return vert;
}

vec4 trace(vec3 ray_origin, vec3 ray_dir) {
    Vertex vert = get_vertex_data(ray_origin, ray_dir);
    return vec4(vert.normal.xyz, 1.0f);
}


layout (binding = 0, rgba32f) uniform image2D framebuffer;
layout (local_size_x = 8, local_size_y = 8) in;

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }

    vec2 tex_coords = vec2(pix)/size;

    vec3 ray = mix(mix(ray00, ray10, tex_coords.x), mix(ray01, ray11, tex_coords.x), tex_coords.y);

    vec4 col = trace(eye, ray);
    // vec4 col;
    // if (pix.x >= 300)
    //     col = vec4(0.0f);
    // else
    //     col = vec4(1.0f);

    imageStore(framebuffer, pix, col);
}
