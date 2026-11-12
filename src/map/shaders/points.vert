#version 440

// The points of a track, each one drawn as a disc in a quad.
// The positions are relative to an origin of the track, in units of the world.

layout(location = 0) in vec2 center;
layout(location = 1) in vec2 corner; // -1 or 1 on each axis

layout(location = 0) out vec2 local; // pixels from the center
layout(location = 1) out float selected;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    vec4 color;
    vec4 borderColor;
    vec4 selectedColor;
    vec4 selectedBorderColor;
    vec4 selection;  // left, top, right, bottom
    vec2 origin;     // top left corner of the view
    float scale;     // pixels per unit of the world
    float radius;    // pixels
    float borderWidth;
};

void main()
{
    local = corner * (radius + 1.0);
    selected = center.x >= selection.x && center.x <= selection.z
            && center.y >= selection.y && center.y <= selection.w ? 1.0 : 0.0;
    gl_Position = qt_Matrix * vec4((center - origin) * scale + local, 0.0, 1.0);
}
