# The map

Marmot draws its map itself, with the Qt Quick scene graph, instead of using the
`Map`/`MapView` items of QtLocation. This document describes how it is put
together, the QML types it provides, and what is still open.

QtLocation is still used, but only for its geocoding and routing models
(`Plugin`, `RouteQuery`, `RouteModel`, `GeocodeModel`). QtPositioning provides
the coordinate types (`QGeoCoordinate`, `QGeoRectangle`).

- [Overview](#overview)
- [QML types](#qml-types)
- [How the application uses them](#how-the-application-uses-them)
- [How it works](#how-it-works)
- [Tile providers](#tile-providers)
- [Performance](#performance)
- [Building](#building)
- [Changes in behaviour](#changes-in-behaviour)
- [Known limits and open points](#known-limits-and-open-points)

## Overview

| File | Role |
|---|---|
| `src/map/mapprojection.h` | Web Mercator projection, normalised to the unit square |
| `src/map/map.h/.cpp` | `Map`: the view (center, zoom level) and the tiles |
| `src/map/maptype.h/.cpp` | `MapType`: a tile source, read from a provider file |
| `src/map/tileloader.h/.cpp` | `TileLoader`: downloads, caches and decodes tiles |
| `src/map/mapitem.h/.cpp` | `MapItem` (base class), `MapMarker`, `MapRectangle` |
| `src/map/maptrack.h/.cpp` | `MapTrack` (line of a track), `MapTrackPoints` (its points) |
| `src/map/shaders/` | Shaders of the track lines and points, and their compiled `.qsb` files |
| `src/qml/MapCanvas.qml` | A `Map` with mouse/touch navigation and the credits of the tiles |

Everything is registered in the `Marmot 1.0` QML module by `src/main.cpp`.

## QML types

### Map

A slippy map: it draws the raster tiles of the active map type and defines the
view which its `MapItem` children use to place themselves. It does not handle
input; `MapCanvas` does.

| Property | Type | Description |
|---|---|---|
| `center` | coordinate | Center of the view |
| `zoomLevel` | real | Zoom level; fractional values are allowed |
| `minimumZoomLevel` | real, read-only | The level at which the world is as high as the item |
| `maximumZoomLevel` | real, read-only | The highest level of the active map type |
| `providersPath` | url | Directory of the provider files (`qrc:` or local) |
| `userAgent` | string | User agent of the tile requests |
| `supportedMapTypes` | list of `MapType`, read-only | The map types found in `providersPath` |
| `activeMapType` | `MapType` | The map type being shown |

| Method | Description |
|---|---|
| `toCoordinate(point)` | Coordinate shown at a position of the item |
| `fromCoordinate(coordinate)` | Position of the item where a coordinate is shown |
| `pan(dx, dy)` | Moves the view by a number of pixels |
| `alignCoordinateToPoint(coordinate, point)` | Moves the view so that the coordinate is shown at the position |
| `fitViewportToGeoShape(shape, margins = 10)` | Shows the bounding rectangle of a geoshape, at a whole zoom level |
| `fitViewportToVisibleMapItems(margins = 10)` | Shows every visible map item; returns `false` if there is none |
| `mapType(key)` | The map type with this key (`"hiking"`, `"satellite"`...), or `null` |
| `mapItemAt(point)` | The topmost visible map item drawn at a position, or `null` |

Signals: one `…Changed` signal per property, and `viewChanged()` when the
center, the zoom level or the size of the map changes.

The view is kept within limits: the zoom level between the minimum and the
maximum, the center between the poles. The longitude wraps around.

### MapType

A tile source. Read-only properties: `key`, `name` (translated), `copyright`
(HTML with links) and `maximumZoomLevel`.

### MapItem

Base class of the items placed geographically (not creatable from QML). A map
item belongs to the `Map` it is a **direct child** of.

### MapMarker

Keeps one of its points on a coordinate. Its content is made of regular child
items.

| Property | Type | Description |
|---|---|---|
| `coordinate` | coordinate | Where the marker is |
| `anchorPoint` | point | The point of the item kept on the coordinate |

`WayPointItem.qml` (a disc) and `PoiMapItem.qml` (a pin and a label) are markers.

### MapRectangle

Covers the area between two coordinates (`topLeft`, `bottomRight`). Its content
is made of regular child items.

### MapTrack

The line of a track.

| Property | Type | Description |
|---|---|---|
| `track` | `Track` | The track to draw |
| `color` | color | Color of the line |
| `lineWidth` | real | Width of the line, in pixels |
| `fragmentShader` | url | A fragment shader which draws the line instead of the default one |
| `shaderPasses` | int | How many times that shader draws the track (1 to 4) |
| `secondaryColor` | color | Passed on to that shader |
| `shaderParameters` | vector4d | Passed on to that shader |

The last four properties are described in [track-shaders.md](track-shaders.md).

`mapItemAt()` returns a `MapTrack` when the position is within 3 pixels of its
line.

### MapTrackPoints

The points of a track, as discs. Used for the file being edited.

| Property | Type | Description |
|---|---|---|
| `track` | `Track` | The track whose points are drawn |
| `color`, `borderColor` | color | Colors of the discs |
| `selectedColor`, `selectedBorderColor` | color | Colors of the discs in the selected area |
| `radius`, `borderWidth` | real | Size of the discs, in pixels |
| `selection` | georectangle | The selected area |

`pointAt(point)` returns the index of the point of the track drawn at a
position of the map, or -1.

### MapCanvas

An interactive map: a `Map` which is dragged, flicked, pinched and zoomed with
the wheel (around the pointer), plus a label with the credits of the tiles.

| Property | Type | Description |
|---|---|---|
| `map` | `Map` | The map |
| `gestureEnabled` | bool | Whether the pointer moves and zooms the map |

The items declared inside a `MapCanvas` are added to its map. Items created
later have to be given `mapCanvas.map` as their parent.

```qml
MapCanvas {
    id: mapView
    map.providersPath: "qrc:/providers/"
    map.zoomLevel: 10

    Marmot.MapTrack {
        track: someTrack
        color: "blue"
        lineWidth: 5
    }
    Marmot.MapMarker {
        coordinate: QtPositioning.coordinate(45.19, 5.72)
        anchorPoint: Qt.point(width/2, height/2)
        width: 10; height: 10
        Rectangle { anchors.fill: parent; radius: 5; color: "red" }
    }
}
```

## How the application uses them

`main.qml` holds a `MapCanvas` (`mapView`) with:

- a `MouseArea` over the map for what is specific to the application: plotting
  a route, selecting a zone, moving and deleting points, the information of a
  track on a right click, fitting a track to the view on a double click;
- the line of the route being plotted, its waypoints and the points of interest;
- the points of the active file (`MapTrackPoints`) and the selection zone
  (`MapRectangle`);
- the popup with the information of a track (`MapMarker`).

The lines of the opened files (`PolyLine.qml`, a `MapTrack`) and their points
of interest are created when a file is appended, with `mapView.map` as parent.

`Track::removePoints(region)` removes every point of a track located in a
georectangle at once; the Delete shortcut uses it for the selection zone.

## How it works

### Projection

`mapprojection.h` converts coordinates to Web Mercator normalised to the unit
square: x and y go from 0 to 1, y pointing south. Everything is computed with
doubles on the CPU. At zoom level z the world is `256 * 2^z` pixels wide.

### Tiles

On every change of the view, `Map::updateTiles()` (called from `updatePolish()`)
lists the tiles to draw:

- the tiles of the zoom level closest to the view's, limited to what the
  provider offers. They are snapped to whole pixels when shown at their native
  size, to stay sharp;
- for a tile which is not loaded yet, a part of a loaded ancestor (up to 5
  levels up), or else its four children if they are loaded.

It then asks `TileLoader` for the missing tiles, closest to the center first,
and cancels the downloads which are not needed any more.

`TileLoader`:

- downloads through `QNetworkAccessManager` with a disk cache of 512 MB in the
  cache directory of the application (`…/tiles`), which is used before the
  network (`PreferCache`);
- decodes the images in the global thread pool, off the GUI thread;
- keeps the most recently used decoded tiles in memory (at least 256, or three
  times what the view shows);
- retries a failed tile after 10 seconds, three times at most.

The scene graph node of the map keeps one texture per tile it has drawn
recently and reuses its image nodes from one frame to the next.

### Track lines and points

A track is projected once, when its path changes. Its geometry is uploaded to
the GPU in units of the world, relative to the center of the track (floats are
not precise enough otherwise), and the view is given to the shaders as two
parameters: the corner of the view and the number of pixels per unit of the
world. **Panning and zooming only update these parameters**; no geometry is
rebuilt or uploaded.

- **Lines** are drawn as one quad per segment. The fragment shader computes the
  distance to the segment, which gives round joins, round ends and smooth edges
  without extra geometry.
- **Points** are drawn as one quad per point. The fragment shader draws the
  disc and its border, and picks the colors of the points in the selection,
  which is also a parameter.
- **Level of detail**: below zoom level 17, the line is rebuilt once per whole
  zoom level, leaving out the points closer than 0.35 pixel to the previous one.
  From level 17 on, every point is drawn.
- **Culling**: a track out of the view has no node at all until it comes back.

The materials are not batched and tell the renderer that they need the full
matrix. Two unused vertices far away make the renderer consider that a track
covers everything, so that it keeps the order in which tracks are drawn.

The shaders are written for Qt's RHI (`#version 440`, compiled to `.qsb`), so
they run on OpenGL, Vulkan, Metal and Direct3D.

### Markers and rectangles

`MapMarker` and `MapRectangle` are plain items: they set their own position
(and size) every time the view changes, on whole pixels for markers.

## Tile providers

A map type is described by a JSON file in the format of the OSM plugin of
QtLocation. The name of the file is the key of the map type.

```json
{
    "UrlTemplate" : "https://tile.example.com/%z/%x/%y.png",
    "MaximumZoomLevel" : 19,
    "MapCopyRight" : "<a href='https://example.com/'>Example</a>",
    "DataCopyRight" : "<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors"
}
```

- `UrlTemplate` accepts `%z`, `%x`, `%y` or `{z}`, `{x}`, `{y}`.
- The keys `street`, `satellite`, `cycle`, `transit`, `night-transit`, `terrain`
  and `hiking` have a translated name and are listed in this order.
- If the directory has no `street` file, a default OpenStreetMap one is added.

The application reads the files embedded in the executable (`providers/`), or,
when the setting `providersUseEmbedded` is false, those of the `providers`
directory of its configuration directory.

## Performance

Measured with 40 tracks of 20,000 points each, in a 1280×800 window, on an
Intel integrated GPU, against the QtLocation version (branch `web`):

| | QtLocation | This map |
|---|---|---|
| Pan, per frame | 51–81 ms | about 6 ms |
| Pan with the points of a track shown, per frame | 84 ms | about 6 ms |
| Wheel zoom step | about 58 ms | 9–13 ms |
| Freeze of the interface while loading the 40 files | 2.1 s | 1.8 s |

Loading is now bound by the parsing of the GPX files in `File`/`Track` (45 to
60 ms for a file of 20,000 points), not by the map.

## Building

- `Marmot.pro` needs the `network` module of Qt in addition to the previous
  ones, and lists the sources of `src/map`.
- qmake cannot compile shaders, so the compiled `.qsb` files are committed. After
  changing a shader, run `src/map/shaders/compile.sh` (it needs `qsb`, from the
  Qt Shader Tools) and commit the result.

## Changes in behaviour

Compared to the QtLocation version:

- A point of the edited file follows the pointer while it is dragged, instead
  of moving when it is released.
- The Delete key removes the selected points only while the selection zone is
  shown.
- Clicking a search result fits the view to its `boundingShape`. The
  `boundingBox` used before does not exist in Qt 6, so nothing happened.
- The credits of the tiles are a label at the bottom left of the map.
- The plugin parameter `osm.mapping.providersrepository.address` is gone: the
  map reads the provider files itself.
- The markers of the profile chart are shown and hidden with `visible` instead
  of being added to and removed from the map.

A fix which is not about the map came along: the bar, the tooltips and the
markers of the profile chart flickered when the pointer was on a tooltip. With
the KDE desktop style, a `Label` handles the hover, so the tooltip took it away
from the area of the chart below it, which hid the tooltip, which gave the
hover back. The hover area of `ProfileChart.qml` is now above the tooltips.

## Known limits and open points

- **Software rendering**: the track lines and points are custom materials, which
  the software backend of Qt Quick does not draw.
- **Translations**: the new strings (names of the map types, credits) are not in
  the `.ts` files yet.
- **Not verified**: routing with the `ors` plugin (the line of a route is drawn
  correctly from a path set by hand), pinch zoom, KML files, and a display
  scale other than 1.
- **Slow start**: a few times, the window took up to 50 seconds to show up. It
  could not be reproduced on demand and its cause is not known. The last thing
  logged before it is the creation of the GeoClue source of `PositionSource`.
- **Statistics**: the aggregated statistics are not refreshed after an edit
  (this was already the case).

The map was checked by running the application offscreen on the GPU and driving
it with a script (screenshots, simulated mouse and keyboard, timings). That
script is not part of the repository.
