#version 440

layout(location = 0) in vec3 local;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    vec4 color;
    vec2 origin;
    float scale;
    float halfWidth;
};

void main()
{
    // Distance to the segment, which gives round caps and smooth edges.
    float dx = local.x - clamp(local.x, 0.0, local.z);
    float distance = length(vec2(dx, local.y));
    fragColor = color * clamp(halfWidth + 0.5 - distance, 0.0, 1.0);
}
