#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Simulation : public core::BaseRenderer {

  private:
    constexpr static std::uint32_t PARTICLE_COUNT = 131072;
    static_assert(!(PARTICLE_COUNT & (PARTICLE_COUNT - 1)),
      "PARTICLE_COUNT must be 2^N because of bitonic mergesort.");

  public:
    Simulation();

    ~Simulation();

    void render(const core::Camera &camera, float dt) override;

    void resize(std::uint32_t width, std::uint32_t height);

  private:
    ::gl::GLuint createVAO(const ::gl::GLuint &vbo) const;

    void deleteVAO(::gl::GLuint handle);

  private:
    ::gl::GLuint programGridInsert_;
    ::gl::GLuint programGridSort_;
    ::gl::GLuint programGridIndexing_;
    ::gl::GLuint programDensityComputation_;
    ::gl::GLuint programPositionUpdate_;
    ::gl::GLuint programRender_;
    ::gl::GLuint bufGridPairs_;
    ::gl::GLuint bufGridIndices_;
    ::gl::GLuint bufPosition1_;
    ::gl::GLuint bufPosition2_;
    ::gl::GLuint bufVelocity1_;
    ::gl::GLuint bufVelocity2_;
    ::gl::GLuint bufDensity1_;
    ::gl::GLuint bufDensity2_;
    ::gl::GLuint bufWallweight_;
    ::gl::GLuint texDistance_;

    ::gl::GLuint vao_;
    bool swapTextures_;
  };
}
