#pragma once

#include <Eigen/Core>

#include "core/SimulationBase.hpp"

namespace flut
{
  class Simulation : public core::SimulationBase
  {
  public:
    struct SimulationOptions
    {
      float gravity[3] = {0.0f, -9.81f, 0.0f};
      float deltaTimeMod = 1.2f;
      std::int32_t colorMode = 0;
      std::int32_t shadingMode = 1;
    };

    struct SimulationTimes
    {
      float gridInsertMs = 0.0f;
      float gridSortMs = 0.0f;
      float gridIndexingMs = 0.0f;
      float densityComputationMs = 0.0f;
      float forceUpdateMs = 0.0f;
      float renderingMs = 0.0f;
    };

  public:
    constexpr static std::uint32_t PARTICLE_COUNT = 2 << 14;
    const Eigen::Matrix<::gl::GLuint, 3, 1> GRID_RES = {128, 32, 8};
    const Eigen::Matrix<::gl::GLfloat, 3, 1> GRID_LEN = {2.5f, 1.5f, 0.5f};
    const Eigen::Matrix<::gl::GLfloat, 3, 1> GRID_ORIGIN = {-1.25f, -0.75f, -0.25f};
    const std::uint32_t GRID_VOXEL_COUNT = GRID_RES(0) * GRID_RES(1) * GRID_RES(2);

  private:
    constexpr static float DT = 0.003f;
    constexpr static float K = 0.3f;
    constexpr static float MASS = 0.02f;
    constexpr static float RANGE = 0.04f;
    constexpr static float VIS_COEFF = 35.0f;
    constexpr static float REST_PRESSURE = 100.0f;
    constexpr static float REST_DENSITY = 998.27f;

  private:
    constexpr static std::uint32_t SMOOTH_ITERATIONS = 30;

  public:
    Simulation(std::uint32_t width, std::uint32_t height);

    ~Simulation() override;

    void preset1();

    void render(const core::Camera& camera, float dt) override;

    void resize(std::uint32_t width, std::uint32_t height);

    SimulationOptions& options();

    const SimulationTimes& times() const;

  private:
    ::gl::GLuint createParticleVAO(::gl::GLuint vboPos, ::gl::GLuint vboCol) const;

    ::gl::GLuint createBBoxVAO(::gl::GLuint vertices, ::gl::GLuint indices) const;

    void deleteVAO(::gl::GLuint handle);

    void loadFileText(const std::string& filePath, std::vector<char>& text) const;

  private:
    std::uint32_t width_;
    std::uint32_t height_;
    std::uint64_t frame_;
    SimulationTimes time_;
    SimulationOptions options_;
    float weightConstViscosity_;
    float weightConstPressure_;
    float weightConstDefault_;
    ::gl::GLuint timerQueries_[2][6];
    ::gl::GLuint programGridInsert_;
    ::gl::GLuint programGridSort_;
    ::gl::GLuint programGridIndexing_;
    ::gl::GLuint programDensityComputation_;
    ::gl::GLuint programForceUpdate_;
    ::gl::GLuint programRenderGeometry_;
    ::gl::GLuint programRenderFlat_;
    ::gl::GLuint programRenderCurvature_;
    ::gl::GLuint programRenderShading_;
    ::gl::GLuint bufBBoxVertices_;
    ::gl::GLuint bufBBoxIndices_;
    ::gl::GLuint bufColor_;
    ::gl::GLuint bufGridUnsorted_;
    ::gl::GLuint bufGridSorted_;
    ::gl::GLuint bufGridIndices_;
    ::gl::GLuint bufPosition1_;
    ::gl::GLuint bufPosition2_;
    ::gl::GLuint bufVelocity1_;
    ::gl::GLuint bufVelocity2_;
    ::gl::GLuint bufDensity_;
    ::gl::GLuint vao1_;
    ::gl::GLuint vao2_;
    ::gl::GLuint vao3_;
    ::gl::GLuint fbo1_;
    ::gl::GLuint fbo2_;
    ::gl::GLuint fbo3_;
    ::gl::GLuint texDepth_;
    ::gl::GLuint64 texDepthHandle_;
    ::gl::GLuint texColor_;
    ::gl::GLuint64 texColorHandle_;
    ::gl::GLuint texTemp1_;
    ::gl::GLuint64 texTemp1Handle_;
    ::gl::GLuint texTemp2_;
    ::gl::GLuint64 texTemp2Handle_;
    bool swapFrame_;
  };
}
