#version 440

// Colors a track according to its altitude. This is also the reference of what
// a fragment shader given to MapTrack.fragmentShader can rely on.
//
// A track is drawn one segment at a time, as a quad which goes halfWidth + 1
// pixels past the segment in every direction. The quads of consecutive segments
// overlap around the point they share, and they are drawn in the order of the
// track: what a segment draws there ends up above what the previous one drew.
// The shader runs once more over the whole track for every extra pass asked for
// with MapTrack.shaderPasses, above the previous passes.
//
// Colors are premultiplied by their alpha.

// Where the fragment is, in the frame of the segment and in pixels:
// x along the segment from its start, y across it, z the length of the segment.
layout(location = 0) in vec3 local;
// The data of the points where the segment starts and ends:
// x: length of the track up to the point, in units of the world (times scale: in pixels)
// y: distance from the start of the track, in meters
// z: altitude, in meters
// w: time since the first point, in seconds (0 for the tracks without time)
layout(location = 1) in vec4 dataStart;
layout(location = 2) in vec4 dataEnd;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    vec4 color;          // MapTrack.color, with the opacity of the item
    vec4 secondaryColor; // MapTrack.secondaryColor, likewise
    vec4 parameters;     // MapTrack.shaderParameters, for the shader to use as it sees fit
    vec4 dataMin;        // lowest data of the track
    vec4 dataMax;        // highest data of the track
    vec2 origin;         // top left corner of the view, in units of the world
    float scale;         // pixels per unit of the world
    float halfWidth;     // half of MapTrack.lineWidth, in pixels
    float pass;          // 0 for the first pass, 1 for the second one...
    float opacity;       // opacity of the item, for the colors the shader comes up with
};

// How much of the fragment a line going this far from the segment covers.
// This is what gives round joins and smooth edges.
float coverage(float radius)
{
    float dx = local.x - clamp(local.x, 0.0, local.z);
    return clamp(radius + 0.5 - length(vec2(dx, local.y)), 0.0, 1.0);
}

// The data at the point of the segment closest to the fragment.
vec4 data()
{
    return mix(dataStart, dataEnd, clamp(local.x / max(local.z, 0.0001), 0.0, 1.0));
}

vec3 ramp(float t)
{
    const vec3 stops[5] = vec3[5](
        vec3(0.16, 0.36, 0.90),
        vec3(0.10, 0.75, 0.80),
        vec3(0.25, 0.75, 0.25),
        vec3(0.95, 0.80, 0.15),
        vec3(0.90, 0.20, 0.15));
    float position = clamp(t, 0.0, 1.0) * 4.0;
    int index = int(min(floor(position), 3.0));
    return mix(stops[index], stops[index + 1], position - float(index));
}

void main()
{
    // parameters.x and y: the altitudes at the ends of the colors. Those of the track by default.
    float low = parameters.x;
    float high = parameters.y;
    if (high <= low) {
        low = dataMin.z;
        high = dataMax.z;
    }
    float t = (data().z - low) / max(high - low, 0.001);
    fragColor = vec4(ramp(t), 1.0) * color.a * coverage(halfWidth);
}
