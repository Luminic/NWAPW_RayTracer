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

struct Mesh {
                          // Base Alignment  // Aligned Offset
    mat4 transformation;  // 16              // 0
                          // 16              // 16
                          // 16              // 32
                          // 16 (total: 64)  // 48

    int material_index;   // 4               // 64
    
    // (PADDING)          // 12              // 68
    // (12 bytes of padding to pad out struct to a multiple of a vec4 because it will be used in an array)

    // Total Size: 80
};

layout (std140, binding=5) buffer MeshBuffer {
    Mesh meshes[];
    //          // Base Alignment  // Aligned Offset
    // mesh[0]  // 80              // 0
    // mesh[1]  // 80              // 80
    // mesh[3]  // 80              // 160
    // ...
};

#define MAX_NR_TEXTURES 3
// From binding 1 (GL_TEXTURE1) to binding MAX_NR_TEXTURES (GL_TEXTURE1 + MAX_NR_TEXTURES)
uniform sampler2D textures[MAX_NR_TEXTURES];

struct Material {
    // textures_index corresponds to textured_materials[textures_index] if the
    // material has textures

                        // Base Alignment  // Aligned Offset
    vec4 albedo;        // 16              // 0
    vec4 F0;            // 16              // 16
    float roughness;    // 4               // 32
    float metalness;    // 4               // 36
    float AO;           // 4               // 40

    // The following ints are texture indices

    int albedo_ti;         // 4               // 44
    int F0_ti;             // 4               // 48
    int roughness_ti;      // 4               // 52
    int metalness_ti;      // 4               // 56
    int AO_ti;             // 4               // 60

    // Total Size: 64
};

layout(std140, binding=6) buffer MaterialBuffer {
    Material materials[];
};

// The per-pixel material data once the textures have been read
// and added to the color information
struct MaterialData {
    vec4 albedo;
    vec4 F0;
    float roughness;
    float metalness;
    float AO;
};

// Basically a texture in all but name
// The data at pixel (u,v) is located at u+v*size_x
layout(std430, binding=7) buffer MeshIndexBuffer {
    int mesh_indices[];
};


MaterialData get_material_data(Material material, vec2 tex_coord) {
    MaterialData material_data = MaterialData(material.albedo, material.F0, material.roughness, material.metalness, material.AO);

    if (material.albedo_ti != -1) {
        material_data.albedo = pow(texture(textures[material.albedo_ti], tex_coord), 2.2f.xxxx);
    }
    if (material.F0_ti != -1) {
        material_data.F0 = texture(textures[material.F0_ti], tex_coord);
    }
    if (material.roughness_ti != -1) {
        material_data.roughness = texture(textures[material.roughness_ti], tex_coord).r;
    }
    if (material.metalness_ti != -1) {
        material_data.metalness = texture(textures[material.metalness_ti], tex_coord).g;
    }
    if (material.AO_ti != -1) {
        material_data.AO = texture(textures[material.AO_ti], tex_coord).b;
    }
    return material_data;
}


uniform vec3 eye;
uniform vec3 ray00;
uniform vec3 ray10;
uniform vec3 ray01;
uniform vec3 ray11;

layout (binding = 0) uniform samplerCube environment_map;

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
    Returns the barycentric coordinates of point if the point is in the triangle
    The w value is 1 if the point is inside of the triangle and -1 otherwise
    */
    float double_area_tri = length(cross(tri1-tri0, tri2-tri0));

    float area0 = length(cross(tri1-point, tri2-point)) / double_area_tri;
    if (area0 > 1+EPSILON) return vec4(0, 0, 0, -1);
    float area1 = length(cross(tri0-point, tri2-point)) / double_area_tri;
    if (area0+area1 > 1+EPSILON) return vec4(0, 0, 0, -1);
    float area2 = length(cross(tri0-point, tri1-point)) / double_area_tri;
    if (area0+area1+area2 > 1+EPSILON) return vec4(0, 0, 0, -1);

    // if (area0+area1+area2-1 <= EPSILON) {
        // If the combined area of the 3 mini triangles equals the area of the triangle
        // the point is inside of the triangle
        return vec4(area0, area1, area2, 1);
    // } else {
    //     // Otherwise, the point is outside of the triangle
    //     // Still return the barycentric coordinates because they might still be useful
    //     return vec4(area0, area1, area2, -1);
    // }
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

float NDF_trowbridge_reitz_GGX(vec3 normal, vec3 halfway, float alpha) {
    float a2 = alpha * alpha;

    return a2 / ( PI * pow( pow(max(dot(normal, halfway), 0.0f), 2) * (a2 - 1) + 1, 2) );
}

float GF_schlick_GGX(float n_dot_v, float roughness) {
    // Schlick approximation
    float k = (roughness+1.0f)*(roughness+1.0f) / 8.0f;

    return n_dot_v / ( n_dot_v * (1-k) + k );
}

float GF_smith(vec3 view, vec3 normal, vec3 light, float alpha) {
    float n_dot_v = max(dot(normal, view), 0.0f);
    float n_dot_l = max(dot(normal, light), 0.0f);
    
    return GF_schlick_GGX(n_dot_v, alpha) * GF_schlick_GGX(n_dot_l, alpha);
}

vec3 F_schlick(vec3 view, vec3 halfway, vec3 F0) {
    // F0 is the reflectivity at normal incidence
    return F0 + (1.0f - F0) * pow((1.0f - max(dot(view, halfway), 0.0f)), 5);
}

vec3 cook_torrance_BRDF(vec3 view, vec3 normal, vec3 light, MaterialData material) {
    vec3 lambertian_diffuse = material.albedo.rgb / PI;

    float alpha = material.roughness * material.roughness;
    vec3 F0 = material.F0.rgb;
    F0 = mix(F0, material.albedo.rgb, material.metalness);
    vec3 halfway = normalize(view + light);

    float NDF = NDF_trowbridge_reitz_GGX(normal, halfway, alpha);
    float GF = GF_smith(view, normal, light, material.roughness);
    vec3 F = F_schlick(view, halfway, F0);

    vec3 kD = (1.0f.xxx - F) * (1.0f - material.metalness);

    vec3 numer = NDF * GF * F;
    float denom = 4.0f * max(dot(normal, view), 0.0f) * max(dot(normal, light), 0.0f);

    return kD*lambertian_diffuse + numer/max(denom, 0.001f);
}

// TODO: make make some of these (especially shadows)
// uniforms so they can be controlled in settings
#define OFFSET 0.0001f
#define SUN_DIR  normalize(vec3(-0.2f, 1.0f, 0.2f))
#define SUN_RADIANCE vec3(3.0f);
#define SHADOWS 1
#define AMBIENT_MULTIPLIER 0.5f
#define BIAS 0.0001f
vec4 shade(Vertex vert, vec3 ray_dir, MaterialData material) {
    vec3 normal = vert.normal.xyz;
    // Make the normal always facing the camera
    normal = normalize(normal) * sign(dot(normal, -ray_dir));

    vec3 radiance = vec3(0.0f);
    #if SHADOWS
        if (get_vertex_data(vert.position.xyz-SUN_DIR*(NEAR_PLANE-BIAS), SUN_DIR).mesh_index == -1)
            radiance += SUN_RADIANCE;
    #else
        radiance += SUN_RADIANCE;
    #endif

    vec3 color = cook_torrance_BRDF(-ray_dir, normal, SUN_DIR, material);
    color *= radiance * max(dot(normal, SUN_DIR), 0.0f);
    
    color += material.albedo.rgb * material.AO * AMBIENT_MULTIPLIER;
    return vec4(color, 1.0f);
}


layout (binding = 0, rgba32f) uniform image2D framebuffer;
layout (binding = 1, rgba32f) uniform image2D geometry;
layout (binding = 2, rgba32f) uniform image2D normals;

subroutine void Trace(vec3 ray_origin, vec3 ray_dir, ivec2 pix, ivec2 size);
subroutine uniform Trace trace;

subroutine(Trace)
void realtime_trace(vec3 ray_origin, vec3 ray_dir, ivec2 pix, ivec2 size) {
    vec4 col;
    vec4 geom;
    vec4 norms;
    Vertex vert = get_vertex_data(ray_origin, ray_dir);
    if (vert.mesh_index == -1) {
        col = texture(environment_map, ray_dir);
        geom = vec4(normalize(ray_dir)*FAR_PLANE, -1.0f);
        norms = vec4(0.0f.xxx, -1.0f);
    } else {
        Material material = materials[meshes[vert.mesh_index].material_index];
        MaterialData material_data = get_material_data(material, vert.tex_coord);

        col = shade(vert, normalize(ray_dir), material_data);
        geom = vec4(vert.position.xyz, 1.0f);
        norms = vec4(normalize(vert.normal.xyz), 1.0f);
    }
    imageStore(framebuffer, pix, col);
    imageStore(geometry, pix, geom);
    imageStore(normals, pix, norms);

    mesh_indices[pix.x+pix.y*size.x] = vert.mesh_index;
}

subroutine(Trace)
void offline_trace(vec3 ray_origin, vec3 ray_dir, ivec2 pix, ivec2 size) {
    vec4 col = imageLoad(framebuffer, pix);
    vec4 pos = imageLoad(geometry, pix);
    vec4 norm = imageLoad(normals, pix);

    int mesh_index = mesh_indices[pix.x+pix.y*size.x]+1;
    imageStore(framebuffer, pix, vec4(mesh_index/50.0f));
}


layout (local_size_x = 8, local_size_y = 8) in;

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }

    vec2 tex_coords = vec2(pix)/size;

    vec3 ray = mix(mix(ray00, ray10, tex_coords.x), mix(ray01, ray11, tex_coords.x), tex_coords.y);

    trace(eye, ray, pix, size);

    // uint prev_rand = uint(gl_GlobalInvocationID.x*size.y+gl_GlobalInvocationID.y);
    // for (int i=0; i<(gl_GlobalInvocationID.y+gl_GlobalInvocationID.x)/10; i++) {
    //     prev_rand = rand(prev_rand);
    // }
}
