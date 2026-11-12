#version 440

// A track with an outline. See line_altitude.frag for what a fragment shader is given.
// It takes two passes (MapTrack.shaderPasses): the outline of a segment would
// otherwise be drawn over the segment before it.
//
// secondaryColor: color of the outline (white if it is transparent)
// parameters.x: width of the outline, in pixels (1.5 by default), taken from the width of the line

layout(location = 0) in vec3 local;
layout(location = 1) in vec4 dataStart;
layout(location = 2) in vec4 dataEnd;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    vec4 color;
    vec4 secondaryColor;
    vec4 parameters;
    vec4 dataMin;
    vec4 dataMax;
    vec2 origin;
    float scale;
    float halfWidth;
    float pass;
    float opacity;
};

float coverage(float radius)
{
    float dx = local.x - clamp(local.x, 0.0, local.z);
    return clamp(radius + 0.5 - length(vec2(dx, local.y)), 0.0, 1.0);
}

void main()
{
    float outline = parameters.x > 0.0 ? parameters.x : 1.5;
    if (pass < 0.5) {
        vec4 outlineColor = secondaryColor.a > 0.0 ? secondaryColor : vec4(opacity);
        fragColor = outlineColor * coverage(halfWidth);
    } else {
        fragColor = color * coverage(halfWidth - outline);
    }
}
