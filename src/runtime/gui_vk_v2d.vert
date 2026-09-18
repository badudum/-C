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

layout(location = 0) out vec4 vCol;

void main()
{
    float x = (inPos.x / u.screen.x) * 2.0 - 1.0;
    float y = (inPos.y / u.screen.y) * 2.0 - 1.0;
    gl_Position = vec4(x, y, 0.0, 1.0);
    vCol = inCol;
}
