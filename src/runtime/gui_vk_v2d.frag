#version 450
layout(location = 0) in vec4 vCol;
layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vCol;
}
