#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <cstdint>
#include <memory>

#include "Camera.hpp"
#include "GlQueryRetriever.hpp"

namespace flut
{
  class Simulation
  {
  public:
    struct SimulationOptions
    {
      float gravity[3] = {0.0f, -9.81f, 0.0f};
      float deltaTimeMod = 1.0f;
      std::int32_t colorMode = 0;
      std::int32_t shadingMode = 1;
    };

    using SimulationTimes = GlQueryRetriever::QueryTimings;

  public:
    constexpr static float DT = 0.0012f;
    constexpr static float STIFFNESS = 250.0;
    constexpr static float MASS = 0.02f;
    constexpr static float PARTICLE_RADIUS = 0.0457f;
    constexpr static float KERNEL_RADIUS = PARTICLE_RADIUS * 4.0f;
    constexpr static float CELL_SIZE = PARTICLE_RADIUS * 4.0f;
    constexpr static float VIS_COEFF = 0.035f;
    constexpr static float REST_DENSITY = 998.27f;
    constexpr static float REST_PRESSURE = 0.0f;
    constexpr static std::uint32_t PARTICLE_COUNT = 100000;

    const glm::vec3 GRID_SIZE = glm::vec3{ 11.0f, 8.0f, 2.5f } * glm::vec3{ 2.0f };
    const glm::vec3 GRID_ORIGIN = GRID_SIZE * -0.5f;
    const glm::ivec3 GRID_RES = glm::ivec3((GRID_SIZE / CELL_SIZE) + 1.0f);
    const std::uint32_t GRID_VOXEL_COUNT = GRID_RES.x * GRID_RES.y * GRID_RES.z;

  private:
    constexpr static std::uint32_t SMOOTH_ITERATIONS = 50;

  public:
    Simulation(std::uint32_t width, std::uint32_t height);

    ~Simulation();

  public:
    void render(const Camera& camera, float dt);

    void resize(std::uint32_t width, std::uint32_t height);

    SimulationOptions& options();

    const SimulationTimes& times() const;

    void setIntegrationsPerFrame(std::uint32_t ipF);

  private:
    void createFrameObjects();

    void deleteFrameObjects();

  private:
    std::uint32_t width_;
    std::uint32_t height_;
    std::uint32_t newWidth_;
    std::uint32_t newHeight_;
    std::uint64_t frame_;
    SimulationTimes time_;
    SimulationOptions options_;
    std::unique_ptr<GlQueryRetriever> queries_;
    std::uint32_t integrationsPerFrame_;
    GLuint programSimStep1_;
    GLuint programSimStep2_;
    GLuint programSimStep3_;
    GLuint programSimStep4_;
    GLuint programSimStep5_;
    GLuint programSimStep6_;
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
    GLuint texVelocity_;
    GLuint64 texVelocityHandle_;
    GLuint64 texVelocityImgHandle_;
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
