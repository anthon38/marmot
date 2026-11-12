#version 440

// A segment of a polyline, drawn as a quad large enough for its round caps.
// The positions are relative to an origin of the polyline, in units of the world.

layout(location = 0) in vec2 segmentStart;
layout(location = 1) in vec2 segmentEnd;
// x: 0 at the start of the segment and 1 at its end, y: -1 or 1 for the side.
layout(location = 2) in vec2 corner;

// Position in pixels in the frame of the segment (x along it, from its start), and its length.
layout(location = 0) out vec3 local;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    vec4 color;
    vec2 origin;     // top left corner of the view
    float scale;     // pixels per unit of the world
    float halfWidth; // pixels
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
    gl_Position = qt_Matrix * vec4(start + direction * along + normal * across, 0.0, 1.0);
}
