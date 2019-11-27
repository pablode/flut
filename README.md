## flut

GPU-based fluid simulation and rendering using OpenGL 4.6 compute shaders, DSA and bindless textures. Fluid behaviour is simulated using _smoothed-particle hydrodynamics_ (SPH) as described by Müller et al. [1]. The GPU simulation pipeline roughly follows the work of Harada et al. [2]. After particle simulation, a screen-space rendering technique is performed to suggest a fluid-like continuous surface [3].  
  
[1] Particle-Based Fluid Simulation for Interactive Applications, Müller et al. 2003  
[2] Smoothed Particle Hydrodynamics on GPUs, Harada et al. 2007  
[3] Screen Space Fluid Rendering with Curvature Flow, van der Laan et al. 2009  

#### Simulation Pipeline Overview

![overview](pipeline.png)

#### Build

This project uses CMake for generating buildsystem files and Git submodules for dependency tracking. Make sure you `git clone` with the `--recursive` flag or execute `git submodule update --init --recursive` after a non-recursive clone.  
Then, invoke CMake for your buildsystem of choice and build the `flut` target.  
Example: `mkdir -p build && cd build && cmake .. -G Ninja && ninja flut`.

#### Future Improvements

- using prefix sums for grid construction is much faster than a sorting-based approach
- memory coherence can be improved by interleaving grouped particle data instead of individual buffers
- group-shared memory can be used to speed up neighborhood search significantly (4x4x4 groups)
- shader code is highly unoptimized and performance can be drastically improved
  - apply loop-unrolling, reduce divergence by avoiding branches, apply neighborhood-search micro-optimizations
- replace curvature flow with ray marching for better visualization (distance-independent, volumetric effects)
- extensions like NV_command_list could improve performance even more
