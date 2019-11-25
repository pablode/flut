## flut

GPU-based fluid simulation and rendering. Utilizes OpenGL 4.3 with Compute Shaders, Eigen and SDL2 to perform _smoothed-particle hydrodynamics_ (SPH) in a parallel manner. Based on work from Harada et. al.

##### Pipeline Overview

![overview](pipeline.png)

##### Dependency setup

On Linux, you can install the corresponding packages instead of building all dependencies yourself (assuming they're not outdated): `sudo pacman -S git gcc cmake eigen glbinding sdl2`.
If problems occur, build them yourself:
```bash
mkdir -p 3rdparty/temp
cd 3rdparty/temp
cmake ..
make help
make <TARGET>
```

##### Build Instructions

Use _CMake_ to generate buildsystem files.
`mkdir -p build && cd build && cmake .. -G Ninja && ninja`
