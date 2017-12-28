#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Simulation : public core::BaseRenderer {

  public:
    struct SimulationOptions {
      SimulationOptions() : gravity{0.0f, 9.81f, 0.0f}, deltaTimeMod{0.2f}, colorMode{2}, shadingMode{0} {}
      float gravity[3];
      float deltaTimeMod;
      std::int32_t colorMode;
      std::int32_t shadingMode;
    };

  private:
    const Eigen::Matrix<::gl::GLuint, 3, 1> GRID_RES = {32, 32, 32};
    const Eigen::Matrix<::gl::GLfloat, 3, 1> GRID_LEN = {1.0f, 1.0f, 1.0f};
    const Eigen::Matrix<::gl::GLfloat, 3, 1> GRID_ORIGIN = {-0.5f, -0.5f, -0.5f};
    const std::uint32_t GRID_VOXEL_COUNT = GRID_RES(0) * GRID_RES(1) * GRID_RES(2);
    const std::uint32_t PARTICLE_COUNT = 2 << 9;

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
