#version 450
layout(location = 0) in vec3 vNrm;
layout(location = 1) in vec4 vCol;
layout(location = 2) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UBO {
    mat4 mvp;
    vec4 light;
    vec4 screen;
} u;

layout(set = 0, binding = 1) uniform sampler2D atlas;

void main()
{
    vec3 L = normalize(u.light.xyz);
    float ndl = max(dot(normalize(vNrm), L), 0.0);
    float amb = 0.28;
    float spec = pow(max(ndl, 0.0), 8.0) * 0.18;
    float lit = amb + ndl * 0.72 + spec;
    vec4 tex = texture(atlas, vUV);
    outColor = vec4(vCol.rgb * tex.rgb * lit, vCol.a);
}
