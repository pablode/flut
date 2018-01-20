#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Simulation : public core::BaseRenderer {

  public:
    struct SimulationOptions {
      SimulationOptions()
        : gravity{0.0f, -9.81f, 0.0f}
        , deltaTimeMod{1.0f}
        , colorMode{1}
        , shadingMode{1} {}
      float gravity[3];
      float deltaTimeMod;
      std::int32_t colorMode;
      std::int32_t shadingMode;
    };

    struct SimulationTime {
      SimulationTime()
        : gridInsertMs{0.0f}
        , gridSortMs{0.0f}
        , gridIndexingMs{0.0f}
        , densityComputationMs{0.0f}
        , forceUpdateMs{0.0f}
        , rendering{0.0f} {}
      float gridInsertMs;
      float gridSortMs;
      float gridIndexingMs;
      float densityComputationMs;
      float forceUpdateMs;
      float rendering;
    };

  public:
    constexpr static std::uint32_t PARTICLE_COUNT = 2 << 14;
    const Eigen::Matrix<GLuint, 3, 1> GRID_RES = {64, 64, 64};
    const Eigen::Matrix<GLfloat, 3, 1> GRID_LEN = {5.0f, 3.0f, 1.0f};
    const Eigen::Matrix<GLfloat, 3, 1> GRID_ORIGIN = {-2.5f, -1.5f, -0.5f};
    const std::uint32_t GRID_VOXEL_COUNT = GRID_RES(0) * GRID_RES(1) * GRID_RES(2);

  private:
    constexpr static float DT = 0.005f;
    constexpr static float K = 800.0f;
    constexpr static float MASS = 0.1f;
    constexpr static float RANGE = 0.075f;
    constexpr static float VIS_COEFF = 3.5f;
    constexpr static float REST_PRESSURE = 250.0f;
    constexpr static float REST_DENSITY = 1000.0f;

  public:
    Simulation();

    ~Simulation();

    void preset1();

    void render(const core::Camera &camera, float dt) override;

    void resize(std::uint32_t width, std::uint32_t height);

    SimulationOptions& options();

    const SimulationTime& time() const;

  private:
    GLuint createVAO(const GLuint &vboPos, const GLuint &vboCol) const;

    void deleteVAO(GLuint handle);

  private:
    std::uint64_t frame_;
    SimulationTime time_;
    SimulationOptions options_;
    GLuint timerQueries_[2][6];
    GLuint programGridInsert_;
    GLuint programGridSort_;
    GLuint programGridIndexing_;
    GLuint programDensityComputation_;
    GLuint programForceUpdate_;
    GLuint programRender_;
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
    bool swapFrame_;
  };
}
