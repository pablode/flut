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
      float deltaTimeMod = 1.0f;
      std::int32_t colorMode = 0;
      std::int32_t shadingMode = 1;
    };

    struct SimulationTimes
    {
      float gridBuildMs = 0.0f;
      float simStep1Ms = 0.0f;
      float simStep2Ms = 0.0f;
      float renderMs = 0.0f;
    };

  private:
    constexpr static float DT = 0.003f;
    constexpr static float STIFFNESS = 30.0f;
    constexpr static float MASS = 0.02f;
    constexpr static float PARTICLE_RADIUS = 0.0457f;
    constexpr static float KERNEL_RADIUS = PARTICLE_RADIUS * 4.0f;
    constexpr static float CELL_RADIUS = PARTICLE_RADIUS * 4.0f;
    constexpr static float VIS_COEFF = 3.5f;
    constexpr static float REST_DENSITY = 998.27f;
    constexpr static float REST_PRESSURE = 0.0f;

  public:
    constexpr static std::uint32_t PARTICLE_COUNT = 8000;

    const Eigen::Matrix<float, 3, 1> GRID_SIZE = Eigen::Vector3f{ 10.0f, 6.0f, 2.0f };
    const Eigen::Matrix<float, 3, 1> GRID_ORIGIN = GRID_SIZE * -0.5f;

    const Eigen::Matrix<std::int32_t, 3, 1> GRID_RES = {
      (std::int32_t) ((GRID_SIZE(0) / CELL_RADIUS) + 1.0f),
      (std::int32_t) ((GRID_SIZE(1) / CELL_RADIUS) + 1.0f),
      (std::int32_t) ((GRID_SIZE(2) / CELL_RADIUS) + 1.0f)
    };

    const std::uint32_t GRID_VOXEL_COUNT = GRID_RES(0) * GRID_RES(1) * GRID_RES(2);

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
    GLuint createParticleVAO(GLuint ssbo) const;

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
    float weightConstKernel_;
    GLuint timerQueries_[2][4];
    GLuint programBuildGrid1_;
    GLuint programBuildGrid2_;
    GLuint programBuildGrid3_;
    GLuint programSimStep1_;
    GLuint programSimStep2_;
    GLuint programRenderGeometry_;
    GLuint programRenderFlat_;
    GLuint programRenderCurvature_;
    GLuint programRenderShading_;
    GLuint bufBBoxVertices_;
    GLuint bufBBoxIndices_;
    GLuint bufParticles1_;
    GLuint bufParticles2_;
    GLuint bufCounters_;
    GLuint texGrid_;
    GLuint64 texGridImgHandle_;
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
