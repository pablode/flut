#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Simulation : public core::BaseRenderer {

  public:
    struct SimulationOptions {
      SimulationOptions() : gravity{0.0f, 9.81f, 0.0f}, deltaTimeMod{1.5f}, mode{0} {}
      float gravity[3];
      float deltaTimeMod;
      std::int32_t mode;
    };

  private:
    constexpr static std::uint32_t PARTICLE_COUNT = 2 << 10;
    constexpr static std::uint32_t GRID_RES_X = 32;
    constexpr static std::uint32_t GRID_RES_Y = 32;
    constexpr static std::uint32_t GRID_RES_Z = 32;
    constexpr static std::uint32_t GRID_VOXEL_COUNT = GRID_RES_X * GRID_RES_Y * GRID_RES_Z;
    static_assert(!(PARTICLE_COUNT & (PARTICLE_COUNT - 1)),
      "PARTICLE_COUNT must be 2^N because of bitonic mergesort.");

  public:
    Simulation();

    ~Simulation();

    void render(const core::Camera &camera, float dt) override;

    void resize(std::uint32_t width, std::uint32_t height);

    SimulationOptions& options();

  private:
    ::gl::GLuint createVAO(const ::gl::GLuint &vbo) const;

    void deleteVAO(::gl::GLuint handle);

  private:
    SimulationOptions options_;
    ::gl::GLuint programGridInsert_;
    ::gl::GLuint programGridSort_;
    ::gl::GLuint programGridIndexing_;
    ::gl::GLuint programDensityComputation_;
    ::gl::GLuint programVelocityUpdate_;
    ::gl::GLuint programPositionUpdate_;
    ::gl::GLuint programRender_;
    ::gl::GLuint bufGridPairs_;
    ::gl::GLuint bufGridIndices_;
    ::gl::GLuint bufPosition1_;
    ::gl::GLuint bufPosition2_;
    ::gl::GLuint bufVelocity1_;
    ::gl::GLuint bufVelocity2_;
    ::gl::GLuint bufDensity_;
    ::gl::GLuint bufWallweight_;
    ::gl::GLuint vao_;
    bool swapTextures_;
  };
}
