#!/bin/sh
# Regenerates the .qsb files, which have to be committed along with the sources
# of the shaders. qsb is part of the Qt Shader Tools.
cd "$(dirname "$0")"
QSB="${QSB:-$(qmake6 -query QT_HOST_BINS)/qsb}"
for shader in *.vert *.frag; do
    "$QSB" --qt6 -o "$shader.qsb" "$shader" || exit 1
done
