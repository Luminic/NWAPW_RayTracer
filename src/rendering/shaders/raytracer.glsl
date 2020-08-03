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
#define DEFAULT_VERTEX Vertex(vec4(0.0f,0.0f,0.0f,-1.0f), vec4(0.0f), vec2(0.0f), -1)

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

    return vec4(area0, area1, area2, 1);
}

bool triangle_intersection(Vertex v0, Vertex v1, Vertex v2, vec3 ray_origin, vec3 ray_dir, float offset, inout float depth, inout Vertex vert) {
    vec3 normal = cross(vec3(v1.position-v0.position), vec3(v2.position-v0.position));
    float rpi = ray_plane_int(ray_origin, ray_dir, v0.position.xyz, normalize(normal));

    // If the ray intersects the triangle
    float dist = rpi*length(ray_dir);
    if (dist >= offset && dist <= depth) {
        vec3 intersection_point = ray_origin + rpi*ray_dir;
        vec4 bc = barycentric_coordinates(intersection_point, v0.position.xyz, v1.position.xyz, v2.position.xyz);
        // If the point is inside of the triangle
        if (bc.w > 0.0f) {
            depth = dist;
            vert.position = vec4(intersection_point, 1.0f);
            vert.normal = bc.x*v0.normal + bc.y*v1.normal + bc.z*v2.normal;
            vert.tex_coord = bc.x*v0.tex_coord + bc.y*v1.tex_coord + bc.z*v2.tex_coord;
            vert.mesh_index = v0.mesh_index;
            return true;
        }
    }
    return false;
}

Vertex get_vertex_data(vec3 ray_origin, vec3 ray_dir, float offset, float max_dist) {
    /*
    Returns an interpolated vertex from the intersection between the ray and the
    nearest triangle it collides with

    If there is no triangle, the mesh_index will be -1
    */
    float depth = max_dist;
    Vertex vert = DEFAULT_VERTEX;
    for (int i=0; i<static_indices.length()/3; i++) {
        Vertex v0 = vertices[static_indices[i*3]];
        Vertex v1 = vertices[static_indices[i*3+1]];
        Vertex v2 = vertices[static_indices[i*3+2]];

        triangle_intersection(v0, v1, v2, ray_origin, ray_dir, offset, depth, vert);
    }
    for (int i=0; i<dynamic_indices.length()/3; i++) {
        Vertex v0 = vertices[dynamic_indices[i*3]   + static_vertices.length()];
        Vertex v1 = vertices[dynamic_indices[i*3+1] + static_vertices.length()];
        Vertex v2 = vertices[dynamic_indices[i*3+2] + static_vertices.length()];

        triangle_intersection(v0, v1, v2, ray_origin, ray_dir, offset, depth, vert);
    }
    return vert;
}

// PBR Shading

#define PI 3.1415926535f
#define TWO_PI 6.28318531f

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

#define SHADOWS 1

struct Light {
    vec3 direction;
    vec3 radiance;
    float ambient_multiplier;
};

// TODO: Make the sun a uniform
#define DEFAULT_SUN Light(normalize(vec3(-0.2f, 1.0f, 0.2f)), vec3(3.0f), 0.5f)
#define BIAS 0.0001f

vec3 calculate_light(vec3 position, vec3 normal, vec3 ray_dir, MaterialData material, Light light) {
    #if SHADOWS
        if (get_vertex_data(position, light.direction, BIAS, FAR_PLANE).mesh_index != -1) {
            return material.albedo.rgb * material.AO * light.ambient_multiplier;
        }
    #endif
    vec3 color = cook_torrance_BRDF(-ray_dir, normal, light.direction, material);
    color *= light.radiance * max(dot(normal, light.direction), 0.0f);
    color += material.albedo.rgb * material.AO * light.ambient_multiplier;
    return color;
}

#define OFFSET 0.0001f
vec4 shade(vec3 position, vec3 normal, vec3 ray_dir, MaterialData material) {
    // Make the normal always facing the camera
    normal = normalize(normal) * sign(dot(normal, -ray_dir));
    
    vec3 color = calculate_light(position, normal, ray_dir, material, DEFAULT_SUN);
    return vec4(color, 1.0f);
}

vec4 offline_shade(vec3 position, vec3 normal, vec3 ray_dir, MaterialData material) {
    // Make the normal always facing the camera
    normal = normalize(normal) * sign(dot(normal, -ray_dir));
    
    vec3 color = calculate_light(position, normal, ray_dir, material, DEFAULT_SUN);
    return vec4(color, 1.0f);
}


layout (binding = 0, rgba32f) uniform image2D framebuffer;
// Stores the x component of tex_coords in the w place
layout (binding = 1, rgba32f) uniform image2D geometry;
// Stores the y component of tex_coords in the w place
layout (binding = 2, rgba32f) uniform image2D normals;

subroutine void Trace(vec3 ray_origin, vec3 ray_dir, ivec2 pix, ivec2 size);
subroutine uniform Trace trace;

subroutine(Trace)
void realtime_trace(vec3 ray_origin, vec3 ray_dir, ivec2 pix, ivec2 size) {
    vec4 col;
    vec4 geom;
    vec4 norms;
    Vertex vert = get_vertex_data(ray_origin, ray_dir, NEAR_PLANE, FAR_PLANE);
    if (vert.mesh_index == -1) {
        col = texture(environment_map, ray_dir);
        geom = vec4(normalize(ray_dir)*FAR_PLANE, 0.0f);
        norms = 0.0f.xxxx;
    } else {
        Material material = materials[meshes[vert.mesh_index].material_index];
        MaterialData material_data = get_material_data(material, vert.tex_coord);

        // col = texture(environment_map, -normalize(vert.normal.xyz)*sign(dot(vert.normal.xyz, ray_dir)));
        col = shade(vert.position.xyz, vert.normal.xyz, normalize(ray_dir), material_data);
        geom = vec4(vert.position.xyz, vert.tex_coord.x);
        norms = vec4(normalize(vert.normal.xyz), vert.tex_coord.y);

        // col = texture(environment_map, normalize(norms.xyz)*sign(dot(norms.xyz, -ray_dir)));
        // col = vec4(max(normalize(norms.xyz)*sign(dot(norms.xyz, -ray_dir)), 0.0f.xxx), 1.0f);
    }
    imageStore(framebuffer, pix, col);
    imageStore(geometry, pix, geom);
    imageStore(normals, pix, norms);

    mesh_indices[pix.x+pix.y*size.x] = vert.mesh_index;
}

// So we can get the average of all iterations with equal weights
uniform int nr_iterations_done;

vec3 uniform_hemisphere_sample(float r1, float r2) {
    // r1 should be in the range 0-2pi (theta)
    // r2 should be in the range 0-1 (will become phi)
    // returns a sample in cartesian coordinates w/ length of 1
    // the hemisphere is oriented towards +z
    float sin_phi = sqrt(-r2 * (r2 - 2.0f));
    float cos_phi = 1-r2;
    return vec3(sin(r1)*sin_phi, cos(r1)*sin_phi, cos_phi);
}

mat3 rotate_a_to_b(vec3 a, vec3 b) {
    // returns a rotation matrix that rotates a to b
    a = normalize(a);
    b = normalize(b);
    // create axis-angle rotation
    vec3 rot_axis = -normalize(cross(a,b));
    // if (abs(rot_axis).x <= EPSILON && abs(rot_axis).y <= EPSILON && abs(rot_axis).z <= EPSILON)
    //     return mat3(1.0f);
    float cos_theta = dot(a, b);
    if (abs(cos_theta-1) <= EPSILON) {
        return mat3(1.0f);
    } else if (abs(cos_theta+1) <= EPSILON) {
        return mat3(1.0f)*-1;
    }
    // convert to quaternion
    float sin_half_theta = sqrt((1-cos_theta)/2); // trig functions are avoided by using trig identities
    float cos_half_theta = sqrt((1+cos_theta)/2);
    // float sin_half_theta = sin(theta/2);
    // float cos_half_theta = cos(theta/2);
    vec4 q = vec4(
        rot_axis * sin_half_theta,
        cos_half_theta
    );
    // convert to rotation matrix
    return mat3(
        1-2*q.y*q.y-2*q.z*q.z, 2*q.x*q.y-2*q.z*q.w,   2*q.x*q.z+2*q.y*q.w,
        2*q.x*q.y+2*q.z*q.w,   1-2*q.x*q.x-2*q.z*q.z, 2*q.y*q.z-2*q.x*q.w,
        2*q.x*q.z-2*q.y*q.w,   2*q.y*q.z+2*q.x*q.w,   1-2*q.x*q.x-2*q.y*q.y
    );
}

subroutine(Trace)
void offline_trace(vec3 ray_origin, vec3 ray_dir, ivec2 pix, ivec2 size) {
    vec4 col = imageLoad(framebuffer, pix);
    vec4 pos = imageLoad(geometry, pix);
    vec4 norm = imageLoad(normals, pix);
    vec2 tex_coord = vec2(pos.w, norm.w);
    vec3 normal = norm.xyz;
    normal = normalize(vec3(normal)) * sign(dot(vec3(normal), -ray_dir));

    int mesh_index = mesh_indices[pix.x+pix.y*size.x];

    if (mesh_index == -1) {
        imageStore(framebuffer, pix, vec4(col.xyz, 1.0f));
        return;
    }

    Material material = materials[meshes[mesh_index].material_index];
    MaterialData material_data = get_material_data(material, tex_coord);

    // vec4 new_col = shade(pos.xyz, norm.xyz, normalize(ray_dir), material_data);

    // uint prev_rand = uint(gl_GlobalInvocationID.x*size.y+gl_GlobalInvocationID.y+nr_iterations_done*11);
    uint prev_rand = rand(nr_iterations_done);
    // for (int i=0; i<(gl_GlobalInvocationID.y+gl_GlobalInvocationID.x)/10; i++) {
    //     prev_rand = rand(prev_rand);
    // }
    float r1 = ((prev_rand >> 3) & uint((1<<16) -1)) / (65535.0f/TWO_PI); // Should be in the range 0-2pi
    prev_rand = rand(prev_rand);
    float r2 = ((prev_rand >> 3) & uint((1<<16) -1)) / 65535.0f; // Should be in the range 0-1

    // hemisphere oriented towards +z
    vec3 sample_dir = uniform_hemisphere_sample(r1,r2);
    // orient the hemisphere to the normal
    sample_dir = rotate_a_to_b(vec3(0.0f,0.0f,1.0f), normal)*sample_dir;

    Vertex vert = get_vertex_data(pos.xyz, sample_dir, BIAS, FAR_PLANE);
    vec3 new_col;
    if (vert.mesh_index == -1) {
        new_col = texture(environment_map, sample_dir).rgb;
        // new_col = 0.0f.xxxx;
        // new_col = vec4(0.0f);
        // new_col = vec4(pos);
    } else {
        // new_col = texture(environment_map, sample_dir);
        // new_col = vec4(sample_dir, 1.0f);
        Material sample_material = materials[meshes[vert.mesh_index].material_index];
        MaterialData sample_material_data = get_material_data(sample_material, vert.tex_coord);
        // return 
        // new_col = sample_material_data.albedo.xyzz;
        new_col = shade(vert.position.xyz, vert.normal.xyz, normalize(sample_dir), sample_material_data).rgb;

        // new_col = (vert.mesh_index+1)/5.0f.xxxx;
    }
    Light light_ray = Light(sample_dir, new_col, 0.1f);
    vec3 light_influence = calculate_light(pos.xyz, normal, ray_dir, material_data, light_ray);
    // position, normal, ray_dir, material, DEFAULT_SUN


    // imageStore(framebuffer, pix, vec4(max(sample_dir,0.0f.xxx), 1.0f));
    // imageStore(framebuffer, pix, new_col);
    imageStore(framebuffer, pix, mix(col, col+vec4(light_influence,1.0f), 1.0f/(nr_iterations_done+1)));
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

    // if (pix.x>500)
    // imageStore(framebuffer, pix, 2/5.0f.xxxx);
    // else
    trace(eye, ray, pix, size);
}
