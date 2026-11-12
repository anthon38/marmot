#version 440

// The vertex shader of the tracks which have their own fragment shader
// (MapTrack.fragmentShader): same as line.vert, plus the data of the track.

layout(location = 0) in vec2 segmentStart;
layout(location = 1) in vec2 segmentEnd;
// x: 0 at the start of the segment and 1 at its end, y: -1 or 1 for the side.
layout(location = 2) in vec2 corner;
// The data of the points at the start and at the end of the segment.
layout(location = 3) in vec4 segmentDataStart;
layout(location = 4) in vec4 segmentDataEnd;

layout(location = 0) out vec3 local;
layout(location = 1) out vec4 dataStart;
layout(location = 2) out vec4 dataEnd;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    vec4 color;
    vec4 secondaryColor;
    vec4 parameters;
    vec4 dataMin;
    vec4 dataMax;
    vec2 origin;     // top left corner of the view
    float scale;     // pixels per unit of the world
    float halfWidth; // pixels
    float pass;
    float opacity;
};

void main()
{
    vec2 start = (segmentStart - origin) * scale;
    vec2 end = (segmentEnd - origin) * scale;
    float len = length(end - start);
    vec2 direction = len > 0.00001 ? (end - start) / len : vec2(1.0, 0.0);
    vec2 normal = vec2(-direction.y, direction.x);

    float reach = halfWidth + 1.0;
    float along = mix(-reach, len + reach, corner.x);
    float across = corner.y * reach;
    local = vec3(along, across, len);
    dataStart = segmentDataStart;
    dataEnd = segmentDataEnd;
    gl_Position = qt_Matrix * vec4(start + direction * along + normal * across, 0.0, 1.0);
}
