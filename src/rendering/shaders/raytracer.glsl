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
#define DEFUALT_VERTEX Vertex(vec4(0.0f,0.0f,0.0f,-1.0f), vec4(0.0f), vec2(0.0f), -1)

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
    // Indices correspond to vertices[dynamic_indices[i] + nr_static_indices]
    int dynamic_indices[];
};


struct Material {
    vec3 albedo;
    vec3 F0;
    float roughness;
    float metalness;
    float AO;
};
#define DEFAULT_MATERIAL Material(vec3(1.0f), vec3(0.05f), 0.3f, 1.0f, 0.5f)
#define MATERIAL(x) Material(x, vec3(0.05f), 0.3f, 1.0f, 0.5f)


uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

layout (binding = 1) uniform samplerCube environment_map;

// Prev_rand should be initialized to the seed once when the this shader is invocated
uint rand(uint prev_rand) {
    // LCG produces terrible results
    // observe how there is clear repetition when doing: vec4 col = vec4(rand(prev_rand).xxxx / float(1<<24));
    // where prev_rand = uint(gl_GlobalInvocationID.x*size.y+gl_GlobalInvocationID.y);
    prev_rand = (12586549*prev_rand + 12435) % uint((1<<24)-1);
    return prev_rand;
}

// Ray-Triangle Intersection

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
        Vertex v0 = vertices[static_indices[i*3]];
        Vertex v1 = vertices[static_indices[i*3+1]];
        Vertex v2 = vertices[static_indices[i*3+2]];

        triangle_intersection(v0, v1, v2, ray_origin, ray_dir, depth, vert);
    }
    for (int i=0; i<dynamic_indices.length()/3; i++) {
        Vertex v0 = vertices[dynamic_indices[i*3]   + static_vertices.length()];
        Vertex v1 = vertices[dynamic_indices[i*3+1] + static_vertices.length()];
        Vertex v2 = vertices[dynamic_indices[i*3+2] + static_vertices.length()];

        triangle_intersection(v0, v1, v2, ray_origin, ray_dir, depth, vert);
    }
    return vert;
}

// PBR Shading

#define PI 3.1415926535f

float NDF_trowbridge_reitz_GGX(vec3 view, vec3 normal, vec3 light, float alpha) {
    vec3 halfway = normalize(view + light);
    float a2 = alpha * alpha;

    return a2 / ( PI * pow( pow(max(dot(normal, halfway), 0.0f), 2) * (a2 - 1) + 1, 2) );
}

float GF_schlick_GGX(float n_dot_v, float alpha) {
    // Schlick approximation
    float k = alpha / 2.0f;

    return n_dot_v / ( n_dot_v * (1-k) + k );
}

float GF_smith(vec3 view, vec3 normal, vec3 light, float alpha) {
    float n_dot_v = max(dot(normal, view), 0.0f);
    float n_dot_l = max(dot(normal, light), 0.0f);
    
    return GF_schlick_GGX(n_dot_v, alpha) * GF_schlick_GGX(n_dot_l, alpha);
}

vec3 F_schlick(vec3 view, vec3 normal, vec3 F0) {
    // F0 is the reflectivity at normal incidence
    return F0 + (1.0f - F0) * pow((1.0f - dot(view, normal)), 5);
}

vec3 cook_torrance_BRDF(vec3 view, vec3 normal, vec3 light, Material material) {
    vec3 lambertian_diffuse = material.albedo / PI;

    float alpha = material.roughness * material.roughness;
    vec3 F0 = material.F0;
    F0 = mix(F0, material.albedo, material.metalness);

    float NDF = NDF_trowbridge_reitz_GGX(view, normal, light, alpha);
    float GF = GF_smith(view, normal, light, alpha);
    vec3 F = F_schlick(view, normal, F0);

    vec3 kD = (1.0f.xxx - F) * (1.0f - material.metalness);

    vec3 numer = NDF * GF * F;
    float denom = 4.0f * max(dot(normal, view), 0.0f) * max(dot(normal, light), 0.0f);

    return kD*lambertian_diffuse + numer/max(denom, 0.001f);
}

#define OFFSET 0.0001f
#define SUN_DIR  normalize(vec3(-0.5f, 1.0f, 0.5f))
#define SUN_RADIANCE vec3(2.0f);
#define SHADOWS 1
#define AMBIENT_MULTIPLIER 0.03
#define BIAS 0.0001f
vec4 shade(Vertex vert, vec3 ray_dir, Material material) {
    Material tmp;
    bool tmp2;
    vec3 normal = vert.normal.xyz;
    // Make the normal always facing the camera
    normal = normalize(normal) * sign(dot(normal, -ray_dir));

    vec3 diffuse = 0.0f.xxx;

    vec3 radiance = vec3(0.0f);
    if (get_vertex_data(vert.position.xyz-SUN_DIR*(NEAR_PLANE-BIAS), SUN_DIR).mesh_index == -1)
        radiance += SUN_RADIANCE;
    // #if SHADOWS
    //     radiance *= shadow_ray(point, SUN_DIR, FAR_PLANE, 32);
    // #endif

    vec3 color = cook_torrance_BRDF(-ray_dir, normal, SUN_DIR, material);
    color *= radiance * max(dot(normal, SUN_DIR), 0.0f);
    
    color += material.albedo * material.AO * AMBIENT_MULTIPLIER;
    return vec4(color, 1.0f);
}

vec4 trace(vec3 ray_origin, vec3 ray_dir) {
    Vertex vert = get_vertex_data(ray_origin, ray_dir);
    if (vert.mesh_index == -1) {
        // return vec4(0.0f.xxx,1.0f);
        return texture(environment_map, ray_dir);
    }
    return shade(vert, ray_dir, MATERIAL(vec3(1.0f,0.0f,0.0f)));
    //return vert.normal;
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
    //     col = trace(eye, ray);
    // else
    //     col = vec4(1.0f);
    uint prev_rand = uint(gl_GlobalInvocationID.x*size.y+gl_GlobalInvocationID.y);
    // uint prev_rand = 5;
    for (int i=0; i<(gl_GlobalInvocationID.y+gl_GlobalInvocationID.x)/10; i++) {
        prev_rand = rand(prev_rand);
    }
    // prev_rand = (prev_rand >> 16);// & uint(1<<30 - 1);
    // vec4 col = vec4(rand(prev_rand).xxxx / float(1<<24));
    // vec4 col = prev_rand.xxxx/float(size.y*size.x);

    imageStore(framebuffer, pix, col);
}
