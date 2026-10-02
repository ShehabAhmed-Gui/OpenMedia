#version 330 core

uniform sampler2D y_tex;
uniform sampler2D u_tex;
uniform sampler2D v_tex;

// Built on the CPU from the frame's colorspace and range.
uniform mat3 colorMatrix;
uniform vec3 colorOffset;

in vec2 v_coord;
out vec4 fragColor;

void main()
{
    vec3 yuv = vec3(texture(y_tex, v_coord).r,
                    texture(u_tex, v_coord).r,
                    texture(v_tex, v_coord).r) + colorOffset;

    fragColor = vec4(clamp(colorMatrix * yuv, 0.0, 1.0), 1.0);
}
