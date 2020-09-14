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
    const Eigen::Matrix<GLuint, 3, 1> GRID_RES = {128, 32, 8};
    const Eigen::Matrix<GLfloat, 3, 1> GRID_LEN = {2.5f, 1.5f, 0.5f};
    const Eigen::Matrix<GLfloat, 3, 1> GRID_ORIGIN = {-1.25f, -0.75f, -0.25f};
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
    GLuint createParticleVAO(GLuint vboPos, GLuint vboCol) const;

    GLuint createBBoxVAO(GLuint vertices, GLuint indices) const;

    void deleteVAO(GLuint handle);

    void loadFileText(const std::string& filePath, std::vector<char>& text) const;

  private:
    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t newWidth_;
    std::uint32_t newHeight_;
    std::uint64_t frame_;
    SimulationTimes time_;
    SimulationOptions options_;
    float weightConstViscosity_;
    float weightConstPressure_;
    float weightConstDefault_;
    GLuint timerQueries_[2][6];
    GLuint programGridInsert_;
    GLuint programGridSort_;
    GLuint programGridIndexing_;
    GLuint programDensityComputation_;
    GLuint programForceUpdate_;
    GLuint programRenderGeometry_;
    GLuint programRenderFlat_;
    GLuint programRenderCurvature_;
    GLuint programRenderShading_;
    GLuint bufBBoxVertices_;
    GLuint bufBBoxIndices_;
    GLuint bufColor_;
    GLuint bufGridUnsorted_;
    GLuint bufGridSorted_;
    GLuint bufGridIndices_;
    GLuint bufPosition1_;
    GLuint bufPosition2_;
    GLuint bufVelocity1_;
    GLuint bufVelocity2_;
    GLuint bufDensity_;
    GLuint vao1_;
    GLuint vao2_;
    GLuint vao3_;
    GLuint fbo1_;
    GLuint fbo2_;
    GLuint fbo3_;
    GLuint texDepth_;
    GLuint64 texDepthHandle_;
    GLuint texColor_;
    GLuint64 texColorHandle_;
    GLuint texTemp1_;
    GLuint64 texTemp1Handle_;
    GLuint texTemp2_;
    GLuint64 texTemp2Handle_;
    bool swapFrame_;
  };
}
