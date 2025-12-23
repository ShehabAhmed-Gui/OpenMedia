#version 450

layout(location = 0) in vec2 vTex;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D texY;
layout(binding = 2) uniform sampler2D texU;
layout(binding = 3) uniform sampler2D texV;

void main()
{
    float y = texture(texY, vTex).r;
    float u = texture(texU, vTex).r - 0.5;
    float v = texture(texV, vTex).r - 0.5;

    vec3 rgb;
    rgb.r = y + 1.402 * v;
    rgb.g = y - 0.344 * u - 0.714 * v;
    rgb.b = y + 1.772 * u;

    fragColor = vec4(rgb, 1.0);
}
