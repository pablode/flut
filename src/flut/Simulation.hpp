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
      float pointScale = 0.75f;
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
    std::uint32_t m_width;
    std::uint32_t m_height;
    std::uint32_t m_newWidth;
    std::uint32_t m_newHeight;
    std::uint64_t m_frame;
    SimulationTimes m_time;
    SimulationOptions m_options;
    std::unique_ptr<GlQueryRetriever> m_queries;
    std::uint32_t m_integrationsPerFrame;
    GLuint m_programSimStep1;
    GLuint m_programSimStep2;
    GLuint m_programSimStep3;
    GLuint m_programSimStep4;
    GLuint m_programSimStep5;
    GLuint m_programSimStep6;
    GLuint m_programRenderGeometry;
    GLuint m_programRenderCurvature;
    GLuint m_programRenderShading;
    GLuint m_bufBBoxVertices;
    GLuint m_bufBBoxIndices;
    GLuint m_bufParticles1;
    GLuint m_bufParticles2;
    GLuint m_bufCounters;
    GLuint m_bufBillboards;
    GLuint m_texGrid;
    GLuint64 m_texGridImgHandle;
    GLuint m_texVelocity;
    GLuint64 m_texVelocityHandle;
    GLuint64 m_texVelocityImgHandle;
    GLuint m_vao1;
    GLuint m_vao2;
    GLuint m_vao3;
    GLuint m_fbo1;
    GLuint m_fbo2;
    GLuint m_fbo3;
    GLuint m_texDepth;
    GLuint64 m_texDepthHandle;
    GLuint m_texColor;
    GLuint64 m_texColorHandle;
    GLuint m_texTemp1;
    GLuint64 m_texTemp1Handle;
    GLuint m_texTemp2;
    GLuint64 m_texTemp2Handle;
    bool m_swapFrame;
  };
}
