#version 450
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNrm;
layout(location = 2) in vec4 inCol;
layout(location = 3) in vec2 inUV;

layout(set = 0, binding = 0) uniform UBO {
    mat4 mvp;
    vec4 light;
    vec4 screen;
} u;

layout(location = 0) out vec3 vNrm;
layout(location = 1) out vec4 vCol;
layout(location = 2) out vec2 vUV;

void main()
{
    vec4 p = u.mvp * vec4(inPos, 1.0);
    p.y = -p.y;
    p.z = (p.z + p.w) * 0.5;
    gl_Position = p;
    vNrm = inNrm;
    vCol = inCol;
    vUV = inUV;
}
