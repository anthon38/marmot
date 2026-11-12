#version 440

layout(location = 0) in vec2 local;
layout(location = 1) in float selected;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    vec4 color;
    vec4 borderColor;
    vec4 selectedColor;
    vec4 selectedBorderColor;
    vec4 selection;
    vec2 origin;
    float scale;
    float radius;
    float borderWidth;
};

void main()
{
    float distance = length(local);
    vec4 fill = mix(color, selectedColor, selected);
    vec4 border = mix(borderColor, selectedBorderColor, selected);
    float inside = clamp(radius - borderWidth + 0.5 - distance, 0.0, 1.0);
    fragColor = mix(border, fill, inside) * clamp(radius + 0.5 - distance, 0.0, 1.0);
}
