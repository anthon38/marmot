# Track shaders

The line of a track (`MapTrack`, see [map.md](map.md)) is drawn by a fragment
shader. By default it is a line of a single color. A track can be given a
fragment shader of its own instead, which decides what the line looks like and
knows what the track is like at each fragment: distance from the start,
altitude, time.

This is groundwork: the application does not use it yet, and tracks look the
same as before until a `fragmentShader` is set.

- [Using a shader](#using-a-shader)
- [The shaders provided](#the-shaders-provided)
- [Writing a shader](#writing-a-shader)
- [Passes](#passes)
- [Limits](#limits)

## Using a shader

```qml
Marmot.MapTrack {
    track: someTrack
    color: someTrack.color
    lineWidth: 8

    fragmentShader: "qrc:/shaders/line_casing.frag.qsb"
    shaderPasses: 2
    secondaryColor: "black"
    shaderParameters: Qt.vector4d(2, 0, 0, 0)
}
```

| Property | Description |
|---|---|
| `fragmentShader` | A **compiled** fragment shader (`.qsb`), from the resources (`qrc:`) or a local file. Empty for the default line. |
| `shaderPasses` | How many times the shader draws the whole track, each pass above the previous one. From 1 (the default) to 4. See [Passes](#passes). |
| `secondaryColor` | A second color for the shader. Transparent by default. |
| `shaderParameters` | Four numbers for the shader to use as it sees fit. Zeros by default. |

`color` and `lineWidth` are passed on as well. The quads the shader draws into
reach `lineWidth/2 + 1` pixels around the line, so a shader cannot draw further
than that.

To give every track of the application a shader, set these properties in
`src/qml/PolyLine.qml`.

If the file does not exist, a warning is printed and the default line is drawn.
If the file exists but is not a valid shader, or does not match what is
described below, Qt prints errors and the track is not drawn.

## The shaders provided

They are in `src/map/shaders/` and in the resources under `qrc:/shaders/`.

| Shader | Look | Passes | `secondaryColor` | `shaderParameters` |
|---|---|---|---|---|
| `line_altitude.frag.qsb` | Colors from blue (low) to red (high) | 1 | — | x, y: altitudes at the ends of the colors. The range of the track if left at 0. |
| `line_dashed.frag.qsb` | Dashes with round ends | 1 | — | x: length of the dashes, y: length of the gaps, in pixels. 6 and 4 times the width by default. |
| `line_casing.frag.qsb` | Line with an outline | 2 | Outline (white if transparent) | x: width of the outline in pixels, 1.5 by default. It is taken from the width of the line. |
| `line_arrows.frag.qsb` | Line with chevrons pointing forwards | 2 | Chevrons (white if transparent) | x: distance between two chevrons in pixels, 12 times the width by default. |

The round ends of the dashes reach half the width of the line into the gaps: a
gap has to be longer than the line is wide to be seen.

The size of the chevrons follows the width of the line.

## Writing a shader

Start from a copy of `src/map/shaders/line_altitude.frag`, which declares
everything a shader is given, and keep these declarations as they are.

### How a track is drawn

A track is drawn one segment at a time. Each segment is a quad which goes
`lineWidth/2 + 1` pixels past the segment in every direction, and the fragment
shader runs for every fragment of that quad.

The quads of consecutive segments overlap around the point they share. They are
drawn in the order of the track, so what a segment draws there ends up above
what the previous segment drew.

### Inputs

```glsl
layout(location = 0) in vec3 local;
layout(location = 1) in vec4 dataStart;
layout(location = 2) in vec4 dataEnd;
```

`local` is where the fragment is, in the frame of the segment, in pixels:

| Component | Meaning |
|---|---|
| `local.x` | Along the segment, from its start. Negative before the start, more than `local.z` past the end. |
| `local.y` | Across the segment, 0 on the line, signed. |
| `local.z` | Length of the segment. |

`dataStart` and `dataEnd` are the data of the points where the segment starts
and ends:

| Component | Meaning |
|---|---|
| `x` | Length of the track up to the point, in units of the world. Multiplied by `scale`, it is in pixels. |
| `y` | Distance from the start of the track, in meters. |
| `z` | Altitude, in meters. |
| `w` | Time since the first point, in seconds. 0 for a track without time. |

A point which lacks an altitude or a time is given those of the previous point.

### Parameters

```glsl
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
```

| Member | Meaning |
|---|---|
| `color` | `MapTrack.color`, premultiplied, with the opacity of the item |
| `secondaryColor` | `MapTrack.secondaryColor`, likewise |
| `parameters` | `MapTrack.shaderParameters` |
| `dataMin`, `dataMax` | Lowest and highest data of the track, component by component |
| `origin` | Top left corner of the view, in units of the world, relative to the center of the track |
| `scale` | Pixels per unit of the world; it doubles with each zoom level |
| `halfWidth` | Half of `MapTrack.lineWidth`, in pixels |
| `pass` | 0 for the first pass, 1 for the second one... |
| `opacity` | Opacity of the item, for the colors the shader comes up with |

### Output

One color, **premultiplied by its alpha**. A color the shader makes up has to be
multiplied by `opacity` (or by `color.a`) to follow the opacity of the item.

### Helpers

Two functions are worth copying from the examples.

How much of the fragment a line going `radius` pixels away from the segment
covers. It gives round joins, round ends and smooth edges:

```glsl
float coverage(float radius)
{
    float dx = local.x - clamp(local.x, 0.0, local.z);
    return clamp(radius + 0.5 - length(vec2(dx, local.y)), 0.0, 1.0);
}
```

The data at the point of the segment closest to the fragment:

```glsl
vec4 data()
{
    return mix(dataStart, dataEnd, clamp(local.x / max(local.z, 0.0001), 0.0, 1.0));
}
```

A plain line is `fragColor = color * coverage(halfWidth);`.

The distance along the track in pixels, for patterns which repeat, is
`data().x * scale`. The slope and the speed of a segment can be worked out from
its two ends: `(dataEnd.z - dataStart.z) / (dataEnd.y - dataStart.y)` and
`(dataEnd.y - dataStart.y) / (dataEnd.w - dataStart.w)`.

### Compiling

Qt loads compiled shaders only:

```sh
qsb --qt6 -o my_shader.frag.qsb my_shader.frag
```

`qsb` is part of the Qt Shader Tools (`/usr/lib/qt6/bin/qsb` on Arch).

For a shader which belongs to the application: put the source in
`src/map/shaders/`, run `src/map/shaders/compile.sh` (it compiles every shader
of the directory), add the `.qsb` file to `src/map/shaders/shaders.qrc`, and
commit both the source and the `.qsb` file.

A shader file which was compiled again is loaded again the next time
`fragmentShader` is set, even to the same file.

## Passes

Because the quads of consecutive segments overlap, a shader which draws
different things at different places of the line runs into trouble around the
points of the track: the outline of a segment, for instance, is drawn over the
inside of the segment before it.

With `shaderPasses: 2`, the whole track is drawn a first time with `pass` at 0,
then a second time above it with `pass` at 1:

- `line_casing.frag` draws the outline color over the full width in the first
  pass, and the line, narrower, in the second one;
- `line_arrows.frag` draws the line in the first pass and the chevrons in the
  second one, only between the two ends of each segment (`local.x` between 0
  and `local.z`), where the segment is the only one to draw.

The passes share the same geometry; an extra pass costs one more draw of the
track.

## Limits

- **Translucent looks**: wherever two segments overlap, a translucent color is
  blended twice, which shows as darker spots at the points of the track. A glow
  runs into this. The route line of the application avoids it for its own
  opacity by being drawn into a layer (`layer.enabled: true`).
- **Slope and speed** are not given to the shader as such. Computed per segment
  from raw GPS points, they are noisy; they would need smoothing first.
- **The data is simplified with the line**: below zoom level 17, the points too
  close to the previous one are left out, and the data is interpolated between
  the points which are kept.
- **Memory**: a track with a shader of its own carries its data in every vertex
  (56 bytes instead of 24). The default line is not affected.
- **Performance**: with 40 tracks of 20,000 points, panning takes 6 to 7 ms per
  frame with the altitude shader or with the arrows (two passes), the same as
  the default line.
- The application has no setting to pick a shader yet.
