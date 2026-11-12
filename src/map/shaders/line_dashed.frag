#version 440

// A dashed track. See line_altitude.frag for what a fragment shader is given.
//
// parameters.x: length of the dashes, in pixels (6 times the width of the line by default)
// parameters.y: length of the gaps, in pixels (4 times the width of the line by default)
//
// The dashes have round ends, which reach half the width of the line into the gaps.

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

vec4 data()
{
    return mix(dataStart, dataEnd, clamp(local.x / max(local.z, 0.0001), 0.0, 1.0));
}

void main()
{
    float dash = parameters.x > 0.0 ? parameters.x : 12.0 * halfWidth;
    float gap = parameters.y > 0.0 ? parameters.y : 8.0 * halfWidth;
    // How far along the track the fragment is, in pixels.
    float along = data().x * scale;
    float position = mod(along, dash + gap);
    float inDash = clamp(position + 0.5, 0.0, 1.0) * clamp(dash - position + 0.5, 0.0, 1.0);
    fragColor = color * inDash * coverage(halfWidth);
}
