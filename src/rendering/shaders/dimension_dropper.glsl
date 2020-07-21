#version 450 core

layout(location=0) in vec4 position;
layout(location=1) in vec4 normal;
layout(location=2) in vec2 tex_coord;

// the easiest way to use this data would be
// to have this in the pipeline and send this
// to the renderer3d shaders
layout(location=0) out vec3 position3d;
layout(location=1) out vec3 normal3d;
layout(location=2) out vec2 _tex_coord;

uniform float slice;

void main() {
    // for now, just throw away the w component
    position3d = position.xyz;
    normal3d = normal.xyz;
    _tex_coord = tex_coord;
}
