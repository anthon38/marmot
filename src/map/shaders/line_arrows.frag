#version 440

// A track with arrows showing its direction. See line_altitude.frag for what a
// fragment shader is given. It takes two passes (MapTrack.shaderPasses): the
// line, then the arrows, which the next segments would otherwise draw over.
//
// secondaryColor: color of the arrows (white if it is transparent)
// parameters.x: distance between two arrows, in pixels (12 times the width of the line by default)

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
    if (pass < 0.5) {
        fragColor = color * coverage(halfWidth);
        return;
    }

    // Only between the ends of the segment: around them, the fragment belongs to its neighbours.
    if (local.x < 0.0 || local.x > local.z) {
        fragColor = vec4(0.0);
        return;
    }
    float spacing = parameters.x > 0.0 ? parameters.x : 24.0 * halfWidth;
    float thickness = max(halfWidth * 0.55, 1.5);
    float reach = halfWidth * 0.85;
    // How far along the track the fragment is, in pixels. Adding the distance to
    // the middle of the line bends the stripes into chevrons which point forwards.
    float along = mix(dataStart.x, dataEnd.x, local.x / max(local.z, 0.0001)) * scale;
    float position = mod(along + abs(local.y), spacing);
    float inArrow = clamp(position + 0.5, 0.0, 1.0) * clamp(thickness - position + 0.5, 0.0, 1.0)
            * clamp(reach - abs(local.y) + 0.5, 0.0, 1.0);
    vec4 arrowColor = secondaryColor.a > 0.0 ? secondaryColor : vec4(opacity);
    fragColor = arrowColor * inArrow;
}
