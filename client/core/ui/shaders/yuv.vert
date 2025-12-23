#version 450

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;

layout(location = 0) out vec2 vTex;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
};

void main()
{
    gl_Position = qt_Matrix * aPosition;
    vTex = aTexCoord;
}
