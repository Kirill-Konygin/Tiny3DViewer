# Tiny3DViewer

A lightweight desktop 3D model viewer built with C++20 and OpenGL 3.3.

Tiny3DViewer can open a model through a native file dialog, orbit and zoom the
camera, select the model, and move it along an axis with a translation gizmo.

## Features

- Native file picker for opening 3D models
- Orbit camera and scroll-wheel zoom
- Model picking with the mouse
- Translation gizmo for moving the model along the X, Y, and Z axes
- Rendering of material base colors and embedded diffuse and normal textures

## Controls

| Input | Action |
| --- | --- |
| `O` | Open a model |
| Left click on the model | Select the model and show the translation gizmo |
| Left drag on a gizmo arrow | Move the model along the selected axis |
| Left drag outside the gizmo | Rotate the camera |
| Mouse wheel | Zoom in or out |
| `Esc` | Exit |

## Requirements

- Git
- CMake 3.20 or newer
- A C++20-compatible compiler
- A GPU and driver supporting OpenGL 3.3

Third-party libraries are bundled with the repository or included as Git
submodules. Depending on the platform, additional development packages required
by GLFW or Native File Dialog Extended may be needed.

## Build and run

Clone the repository together with its submodules:

```sh
git clone --recursive https://github.com/Kirill-Konygin/Tiny3DViewer.git
```

If the repository was cloned without `--recursive`, initialize the submodules:

```sh
git submodule update --init --recursive
```

Configure and build a release version:

```sh
cmake -S . -B build
cmake --build build --config Release
```

After building with Visual Studio on Windows, run:

```powershell
.\build\Release\Tiny3DViewer.exe
```

## Dependencies

- GLFW for window creation and input
- GLAD for loading OpenGL functions
- GLM for mathematics
- Assimp for model importing
- stb_image for texture decoding
- Native File Dialog Extended for the open-file dialog

## Current limitations

- Only one model can be displayed at a time.
- The gizmo supports translation only; rotation and scaling are not available.
- Material textures referenced as external files are not currently loaded;
  embedded diffuse and normal textures are supported.
- Model animations are not played.
