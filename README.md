# flut

GPU-based fluid simulation and rendering using OpenGL 4.6 compute shaders, DSA and bindless textures. Fluid behaviour is simulated using _smoothed-particle hydrodynamics_ (SPH) as described by Müller et al. [1]. The GPU simulation pipeline roughly follows the work of Harada et al. [2]. After particle simulation, a screen-space rendering technique is performed to suggest a fluid-like continuous surface [3].  

[1] Particle-Based Fluid Simulation for Interactive Applications, Müller et al. 2003  
[2] Smoothed Particle Hydrodynamics on GPUs, Harada et al. 2007  
[3] Screen Space Fluid Rendering with Curvature Flow, van der Laan et al. 2009  

https://github.com/user-attachments/assets/56878c47-0011-4204-90dd-7f5a3ff81442

## Simulation pipeline

flut first builds a uniform grid to speed up SPH neighbor search.
In an earlier version, bitonic mergesort was used for this purpose.
It got later replaced by counting sort, as outlined below.
After grid construction, per-particle densities are computed.
From these values, forces are derived which contribute to velocity and position changes.
Lastly, the particles are splatted as screen space spheres, smoothed to evoke the look of a surface and shaded using Phong.

<img width=800 src="https://github.com/user-attachments/assets/9d085dd2-2f02-4022-b533-58bb9cd5605e" />
<img width=800 src="https://github.com/user-attachments/assets/d13e0533-f8ea-4a6a-8099-d30dfc130a7e" />

### Fast uniform grid construction

To speed up uniform grid construction, counting sort ([Hoetzlein 2024](https://ramakarl.com/pdfs/2014_Hoetzlein_FastFixedRadius_Neighbors.pdf)) is used.
The implementation follows closely what Sebastian Aaltonen described to me about Claybook's implementation.
Particles are first splatted to a 3D grid to obtain the cell particle count.
Next, a reduction is performed to obtain per-cell particle offsets.
Finally, the particles are copied to a new buffer in cell-local order for optimal memory locality.

### Minor optimizations

* The neighborhood search uses an unrolled single loop with interleaved particle fetching as described [here](https://x.com/SebAaltonen/status/1270613495768330241)
* Curvature flow fragment shader is only executed on oriented bounding box
* Very small but nice: negative grid bounds checks are avoided using `uvec3` cast (from [this talk](https://www.graphicsprogrammingconference.nl/realtime-fluid-simulations/))

## Build

Make sure you `git clone` with the `--recursive` flag or execute `git submodule update --init --recursive` after a non-recursive clone.

Then, invoke CMake for your buildsystem of choice and build the `flut` target.  
  
Example:
```sh
mkdir -p BUILD && cd BUILD
cmake .. -G "Visual Studio 16 2019 Win64" -DCMAKE_BUILD_TYPE=Release
cmake --build . -j 8 --target flut --config Release
```

## Future improvements

- Improved rendering
- Boundary density contribution
- Surface tension forces
- BDF-2 integration

## Acknowledgements

Special thanks to Sebastian Aaltonen for his detailed description of Claybook's SPH implementation.

## License

```

    Copyright (C) 2020 Pablo Delgado Krämer

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.

```
