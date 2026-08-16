# Tiny3DViewer

A lightweight 3D model viewer built with C++ and OpenGL 3.3.

## Controls
- `O` - open a model
- Left click on the model - select it and show the translate gizmo
- Left mouse drag on a gizmo arrow - move the model along that axis
- Left mouse drag outside the gizmo - rotate the camera
- Mouse wheel - zoom
- `Esc` - exit

## Installation
`git clone --recursive https://github.com/Kirill-Konygin/Tiny3DViewer.git`
- cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
- cmake --build build --config Release
