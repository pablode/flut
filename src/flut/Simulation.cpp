#include "Simulation.hpp"
#include "GlHelper.hpp"
#include "GlQueryRetriever.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>

using namespace flut;

struct Particle
{
  float position_x;
  float position_y;
  float position_z;
  float density;
  float velocity_x;
  float velocity_y;
  float velocity_z;
  float pressure;
};

Simulation::Simulation(std::uint32_t width, std::uint32_t height)
  : m_width(width)
  , m_height(height)
  , m_newWidth(width)
  , m_newHeight(height)
  , m_swapFrame{false}
  , m_frame{0}
  , m_integrationsPerFrame{1}
{
#ifndef NDEBUG
  GlHelper::enableDebugHooks();
#endif

  // Shaders
  {
    glm::vec3 invCellSize = glm::vec3(GRID_RES) * (1.0f - 0.001f) / GRID_SIZE;

    float viscosityKernelWeightConst = static_cast<float>(45.0f / (M_PI * std::pow(KERNEL_RADIUS, 6)));
    float pressureKernelWeightConst = static_cast<float>(45.0f / (M_PI * std::pow(KERNEL_RADIUS, 6)));
    float densityKernelWeightConst = static_cast<float>(315.0f / (64.0f * M_PI * std::pow(KERNEL_RADIUS, 9)));

    m_programSimStep1 = GlHelper::createComputeShader(RESOURCES_DIR "/simStep1.comp", {
      { "INV_CELL_SIZE",  invCellSize },
      { "GRID_ORIGIN",    GRID_ORIGIN },
      { "PARTICLE_COUNT", PARTICLE_COUNT },
      { "GRID_SIZE",      GRID_SIZE }
    });

    m_programSimStep2 = GlHelper::createComputeShader(RESOURCES_DIR "/simStep2.comp", {
      { "GRID_RES",       GRID_RES }
    });

    m_programSimStep3 = GlHelper::createComputeShader(RESOURCES_DIR "/simStep3.comp", {
      { "INV_CELL_SIZE",  invCellSize },
      { "GRID_ORIGIN",    GRID_ORIGIN },
      { "PARTICLE_COUNT", PARTICLE_COUNT }
    });

    m_programSimStep4 = GlHelper::createComputeShader(RESOURCES_DIR "/simStep4.comp", {
      { "GRID_RES",       GRID_RES }
    });

    m_programSimStep5 = GlHelper::createComputeShader(RESOURCES_DIR "/simStep5.comp", {
      { "INV_CELL_SIZE",               invCellSize },
      { "GRID_ORIGIN",                 GRID_ORIGIN },
      { "GRID_RES",                    GRID_RES },
      { "PARTICLE_COUNT",              PARTICLE_COUNT },
      { "MASS",                        MASS },
      { "KERNEL_RADIUS",               KERNEL_RADIUS },
      { "DENSITY_KERNEL_WEIGHT_CONST", densityKernelWeightConst },
      { "STIFFNESS_K",                 STIFFNESS },
      { "REST_DENSITY",                REST_DENSITY },
      { "REST_PRESSURE",               REST_PRESSURE }
    });

    m_programSimStep6 = GlHelper::createComputeShader(RESOURCES_DIR "/simStep6.comp", {
      { "INV_CELL_SIZE",               invCellSize },
      { "GRID_SIZE",                   GRID_SIZE },
      { "GRID_ORIGIN",                 GRID_ORIGIN },
      { "GRID_RES",                    GRID_RES },
      { "PARTICLE_COUNT",              PARTICLE_COUNT },
      { "MASS",                        MASS },
      { "KERNEL_RADIUS",               KERNEL_RADIUS },
      { "VIS_COEFF",                   VIS_COEFF },
      { "VIS_KERNEL_WEIGHT_CONST",     viscosityKernelWeightConst },
      { "PRESS_KERNEL_WEIGHT_CONST",   pressureKernelWeightConst }
    });

    m_programRenderGeometry = GlHelper::createVertFragShader(RESOURCES_DIR "/renderGeometry.vert", RESOURCES_DIR "/renderGeometry.frag");
    m_programRenderCurvature = GlHelper::createVertFragShader(RESOURCES_DIR "/renderBoundingBox.vert", RESOURCES_DIR "/renderCurvature.frag");
    m_programRenderShading = GlHelper::createVertFragShader(RESOURCES_DIR "/renderBoundingBox.vert", RESOURCES_DIR "/renderShading.frag");
  }

  // Bounding box
  const std::vector<glm::vec3> bboxVertices{
    GRID_ORIGIN + glm::vec3{       0.0f,        0.0f, GRID_SIZE.z},
    GRID_ORIGIN + glm::vec3{GRID_SIZE.x,        0.0f, GRID_SIZE.z},
    GRID_ORIGIN + glm::vec3{GRID_SIZE.x, GRID_SIZE.y, GRID_SIZE.z},
    GRID_ORIGIN + glm::vec3{       0.0f, GRID_SIZE.y, GRID_SIZE.z},
    GRID_ORIGIN + glm::vec3{       0.0f,        0.0f,        0.0f},
    GRID_ORIGIN + glm::vec3{GRID_SIZE.x,        0.0f,        0.0f},
    GRID_ORIGIN + glm::vec3{GRID_SIZE.x, GRID_SIZE.y,        0.0f},
    GRID_ORIGIN + glm::vec3{       0.0f, GRID_SIZE.y,        0.0f},
  };
  glCreateBuffers(1, &m_bufBBoxVertices);
  glNamedBufferStorage(m_bufBBoxVertices, bboxVertices.size() * sizeof(float) * 3, glm::value_ptr(bboxVertices.data()[0]), 0);

  const std::vector<std::uint32_t> bboxIndices {
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4,
    4, 5, 1, 1, 0, 4,
    3, 2, 6, 6, 7, 3
  };
  glCreateBuffers(1, &m_bufBBoxIndices);
  glNamedBufferStorage(m_bufBBoxIndices, bboxIndices.size() * sizeof(std::uint32_t), bboxIndices.data(), 0);

  glCreateVertexArrays(1, &m_vao3);
  glEnableVertexArrayAttrib(m_vao3, 0);
  glVertexArrayVertexBuffer(m_vao3, 0, m_bufBBoxVertices, 0, 3 * sizeof(float));
  glVertexArrayAttribBinding(m_vao3, 0, 0);
  glVertexArrayAttribFormat(m_vao3, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glEnableVertexArrayAttrib(m_vao3, 1);
  glVertexArrayElementBuffer(m_vao3, m_bufBBoxIndices);
  glVertexArrayAttribBinding(m_vao3, 1, 0);
  glVertexArrayAttribFormat(m_vao3, 1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

  // Uniform grid
  glCreateTextures(GL_TEXTURE_3D, 1, &m_texGrid);
  glTextureStorage3D(m_texGrid, 1, GL_R32UI, GRID_RES.x, GRID_RES.y, GRID_RES.z);
  m_texGridImgHandle = glGetImageHandleARB(m_texGrid, 0, GL_FALSE, 0, GL_R32UI);
  glMakeImageHandleResidentARB(m_texGridImgHandle, GL_READ_WRITE);

  glCreateBuffers(1, &m_bufCounters);
  glNamedBufferStorage(m_bufCounters, 4, nullptr, GL_DYNAMIC_STORAGE_BIT);

  // Velocity texture
  glCreateTextures(GL_TEXTURE_3D, 1, &m_texVelocity);
  glTextureParameteri(m_texVelocity, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_texVelocity, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureStorage3D(m_texVelocity, 1, GL_RGBA32F, GRID_RES.x, GRID_RES.y, GRID_RES.z);
  m_texVelocityHandle = glGetTextureHandleARB(m_texVelocity);
  glMakeTextureHandleResidentARB(m_texVelocityHandle);
  m_texVelocityImgHandle = glGetImageHandleARB(m_texVelocity, 0, GL_FALSE, 0, GL_RGBA32F);
  glMakeImageHandleResidentARB(m_texVelocityImgHandle, GL_READ_WRITE);

  // Billboards index buffer
  {
    uint32_t billboardIndexCount = 6;
    uint32_t billboardVertexCount = 4;
    uint32_t billboardIndices[] = { 0, 1, 2, 2, 1, 3 };

    std::vector<uint32_t> indices(billboardIndexCount * PARTICLE_COUNT);

    for (uint32_t i = 0; i < indices.size(); i++)
    {
      uint32_t particleOffset = i / billboardIndexCount;
      uint32_t particleIndexOffset = i % billboardIndexCount;
      indices[i] = billboardIndices[particleIndexOffset] + particleOffset * billboardVertexCount;
    }

    glCreateBuffers(1, &m_bufBillboards);
    glNamedBufferData(m_bufBillboards, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);
  }

  // Initial particles
  std::vector<Particle> particles;
  particles.resize(PARTICLE_COUNT);
  for (std::uint32_t i = 0; i < PARTICLE_COUNT; ++i)
  {
    Particle& p = particles[i];
    const float x = ((std::rand() % 10000) / 10000.0f) * (GRID_SIZE.x * 0.5);
    const float y = ((std::rand() % 10000) / 10000.0f) * (GRID_SIZE.y * 0.5);
    const float z = ((std::rand() % 10000) / 10000.0f) * (GRID_SIZE.z * 0.5);
    p.position_x = GRID_ORIGIN.x + GRID_SIZE.x * 0.25f + x;
    p.position_y = GRID_ORIGIN.y + GRID_SIZE.y * 0.25f + y;
    p.position_z = GRID_ORIGIN.z + GRID_SIZE.z * 0.25f + z;
    p.density = 0.0f;
    p.velocity_x = 0.0f;
    p.velocity_y = 0.0f;
    p.velocity_z = 0.0f;
    p.pressure = 0.0f;
  }

  const auto size = PARTICLE_COUNT * sizeof(Particle);
  glCreateBuffers(1, &m_bufParticles1);
  glCreateBuffers(1, &m_bufParticles2);
  glNamedBufferStorage(m_bufParticles1, size, particles.data(), 0);
  glNamedBufferStorage(m_bufParticles2, size, particles.data(), 0);

  glCreateVertexArrays(1, &m_vao1);
  glEnableVertexArrayAttrib(m_vao1, 0);
  glVertexArrayVertexBuffer(m_vao1, 0, m_bufParticles1, 0, sizeof(Particle));
  glVertexArrayAttribBinding(m_vao1, 0, 0);
  glVertexArrayAttribFormat(m_vao1, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayElementBuffer(m_vao1, m_bufBillboards);

  glCreateVertexArrays(1, &m_vao2);
  glEnableVertexArrayAttrib(m_vao2, 0);
  glVertexArrayVertexBuffer(m_vao2, 0, m_bufParticles2, 0, sizeof(Particle));
  glVertexArrayAttribBinding(m_vao2, 0, 0);
  glVertexArrayAttribFormat(m_vao2, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayElementBuffer(m_vao2, m_bufBillboards);

  // Default state
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  // Textures and buffers
  createFrameObjects();

  // Timer queries
  m_queries = std::make_unique<GlQueryRetriever>();
}

void flut::Simulation::createFrameObjects()
{
  glCreateTextures(GL_TEXTURE_2D, 1, &m_texDepth);
  glTextureStorage2D(m_texDepth, 1, GL_DEPTH_COMPONENT24, m_width, m_height);
  glTextureParameteri(m_texDepth, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_texDepth, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  m_texDepthHandle = glGetTextureHandleARB(m_texDepth);
  glMakeTextureHandleResidentARB(m_texDepthHandle);

  glCreateTextures(GL_TEXTURE_2D, 1, &m_texColor);
  glTextureStorage2D(m_texColor, 1, GL_RGB32F, m_width, m_height);
  glTextureParameteri(m_texColor, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_texColor, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  m_texColorHandle = glGetTextureHandleARB(m_texColor);
  glMakeTextureHandleResidentARB(m_texColorHandle);

  glCreateTextures(GL_TEXTURE_2D, 1, &m_texTemp1);
  glTextureStorage2D(m_texTemp1, 1, GL_R32F, m_width, m_height);
  glTextureParameteri(m_texTemp1, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_texTemp1, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  m_texTemp1Handle = glGetTextureHandleARB(m_texTemp1);
  glMakeTextureHandleResidentARB(m_texTemp1Handle);

  glCreateTextures(GL_TEXTURE_2D, 1, &m_texTemp2);
  glTextureStorage2D(m_texTemp2, 1, GL_R32F, m_width, m_height);
  glTextureParameteri(m_texTemp2, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_texTemp2, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  m_texTemp2Handle = glGetTextureHandleARB(m_texTemp2);
  glMakeTextureHandleResidentARB(m_texTemp2Handle);

  glCreateFramebuffers(1, &m_fbo1);
  glNamedFramebufferTexture(m_fbo1, GL_DEPTH_ATTACHMENT, m_texDepth, 0);
  glNamedFramebufferTexture(m_fbo1, GL_COLOR_ATTACHMENT0, m_texColor, 0);

  glCreateFramebuffers(1, &m_fbo2);
  glNamedFramebufferTexture(m_fbo2, GL_COLOR_ATTACHMENT0, m_texTemp1, 0);

  glCreateFramebuffers(1, &m_fbo3);
  glNamedFramebufferTexture(m_fbo3, GL_COLOR_ATTACHMENT0, m_texTemp2, 0);
}

void flut::Simulation::deleteFrameObjects()
{
  glDeleteFramebuffers(1, &m_fbo1);
  glDeleteFramebuffers(1, &m_fbo2);
  glDeleteFramebuffers(1, &m_fbo3);

  glMakeTextureHandleNonResidentARB(m_texDepthHandle);
  glDeleteTextures(1, &m_texDepth);

  glMakeTextureHandleNonResidentARB(m_texColorHandle);
  glDeleteTextures(1, &m_texColor);

  glMakeTextureHandleNonResidentARB(m_texTemp1Handle);
  glDeleteTextures(1, &m_texTemp1);

  glMakeTextureHandleNonResidentARB(m_texTemp2Handle);
  glDeleteTextures(1, &m_texTemp2);
}

Simulation::~Simulation()
{
  deleteFrameObjects();
  glDeleteProgram(m_programSimStep1);
  glDeleteProgram(m_programSimStep2);
  glDeleteProgram(m_programSimStep3);
  glDeleteProgram(m_programSimStep5);
  glDeleteProgram(m_programSimStep6);
  glDeleteProgram(m_programRenderGeometry);
  glDeleteProgram(m_programRenderCurvature);
  glDeleteProgram(m_programRenderShading);
  glDeleteBuffers(1, &m_bufBBoxVertices);
  glDeleteBuffers(1, &m_bufBBoxIndices);
  glDeleteBuffers(1, &m_bufParticles1);
  glDeleteBuffers(1, &m_bufParticles2);
  glMakeImageHandleNonResidentARB(m_texGridImgHandle);
  glDeleteTextures(1, &m_texGrid);
  glMakeImageHandleNonResidentARB(m_texVelocityImgHandle);
  glMakeTextureHandleNonResidentARB(m_texVelocityHandle);
  glDeleteTextures(1, &m_texVelocity);
  glDeleteBuffers(1, &m_bufCounters);
  glDeleteVertexArrays(1, &m_vao1);
  glDeleteVertexArrays(1, &m_vao2);
  glDeleteVertexArrays(1, &m_vao3);
}

void Simulation::render(const Camera& camera, float dt)
{
  ++m_frame;

  // Resize window if needed.
  if (m_width != m_newWidth || m_height != m_newHeight)
  {
    m_width = m_newWidth;
    m_height = m_newHeight;
    deleteFrameObjects();
    createFrameObjects();
  }

  for (std::uint32_t f = 0; f < m_integrationsPerFrame; f++)
  {
    float dt = DT * m_options.deltaTimeMod;

    // Step 1: Integrate position, do boundary handling.
    //         Write particle count to voxel grid.
    m_queries->beginSimQuery(0);
    const std::uint32_t fClearValue = 0;
    glClearTexImage(m_texGrid, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &fClearValue);
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    glUseProgram(m_programSimStep1);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_swapFrame ? m_bufParticles1 : m_bufParticles2);
    glProgramUniformHandleui64ARB(m_programSimStep1, 0, m_texGridImgHandle);
    glProgramUniform1f(m_programSimStep1, 1, dt);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    m_queries->endQuery();

    // Step 2: Write global particle array offsets into voxel grid.
    m_queries->beginSimQuery(1);
    const std::uint32_t uiClearValue = 0;
    glClearNamedBufferData(m_bufCounters, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &uiClearValue);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glUseProgram(m_programSimStep2);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_bufCounters);
    glProgramUniformHandleui64ARB(m_programSimStep2, 0, m_texGridImgHandle);
    glDispatchCompute(
      (GRID_RES.x + 4 - 1) / 4,
      (GRID_RES.y + 4 - 1) / 4,
      (GRID_RES.z + 4 - 1) / 4
    );
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    m_queries->endQuery();

    // Step 3: Write particles to new location in second particle buffer.
    //         Write particle count to voxel grid (again).
    m_queries->beginSimQuery(2);
    glUseProgram(m_programSimStep3);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_swapFrame ? m_bufParticles1 : m_bufParticles2);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_swapFrame ? m_bufParticles2 : m_bufParticles1);
    glProgramUniformHandleui64ARB(m_programSimStep3, 0, m_texGridImgHandle);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    m_queries->endQuery();

    // Step 4: Write average voxel velocities into second 3D-texture.
    m_queries->beginSimQuery(3);
    glUseProgram(m_programSimStep4);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_swapFrame ? m_bufParticles2 : m_bufParticles1);
    glProgramUniformHandleui64ARB(m_programSimStep4, 0, m_texGridImgHandle);
    glProgramUniformHandleui64ARB(m_programSimStep4, 1, m_texVelocityImgHandle);
    glDispatchCompute(
      (GRID_RES.x + 4 - 1) / 4,
      (GRID_RES.y + 4 - 1) / 4,
      (GRID_RES.z + 4 - 1) / 4
    );
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    m_queries->endQuery();

    // Step 5: Compute density and pressure for each particle.
    m_queries->beginSimQuery(4);
    glUseProgram(m_programSimStep5);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_swapFrame ? m_bufParticles2 : m_bufParticles1);
    glProgramUniformHandleui64ARB(m_programSimStep5, 0, m_texGridImgHandle);
    glDispatchCompute((PARTICLE_COUNT + 64 - 1) / 64, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_queries->endQuery();

    // Step 6: Compute pressure and viscosity forces, use them to write new velocity.
    //         For the old velocity, we use the coarse 3d-texture and do trilinear HW filtering.
    m_queries->beginSimQuery(5);
    glUseProgram(m_programSimStep6);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_swapFrame ? m_bufParticles2 : m_bufParticles1);
    glProgramUniformHandleui64ARB(m_programSimStep6, 0, m_texGridImgHandle);
    glProgramUniformHandleui64ARB(m_programSimStep6, 1, m_texVelocityHandle);
    glProgramUniform1f(m_programSimStep6, 2, dt);
    glProgramUniform3fv(m_programSimStep6, 3, 1, &m_options.gravity[0]);
    glDispatchCompute((PARTICLE_COUNT + 64 - 1) / 64, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    m_queries->endQuery();

    m_swapFrame = !m_swapFrame;
    m_queries->incSimIter();
  }

  // Step 7: Render the geometry as screen-space spheres.
  m_queries->beginRenderQuery();
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo1);
  glUseProgram(m_programRenderGeometry);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  const float pointRadius = KERNEL_RADIUS * m_options.pointScale;
  const auto& view = camera.view();
  const auto& projection = camera.projection();
  const auto& invProjection = camera.invProjection();
  const glm::mat4 vp = projection * view;
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_swapFrame ? m_bufParticles2 : m_bufParticles1);
  glProgramUniformMatrix4fv(m_programRenderGeometry, 0, 1, GL_FALSE, glm::value_ptr(vp));
  glProgramUniformMatrix4fv(m_programRenderGeometry, 1, 1, GL_FALSE, glm::value_ptr(view));
  glProgramUniformMatrix4fv(m_programRenderGeometry, 2, 1, GL_FALSE, glm::value_ptr(projection));
  glProgramUniform3fv(m_programRenderGeometry, 3, 1, glm::value_ptr(GRID_SIZE));
  glProgramUniform3fv(m_programRenderGeometry, 4, 1, glm::value_ptr(GRID_ORIGIN));
  glProgramUniform3iv(m_programRenderGeometry, 5, 1, glm::value_ptr(GRID_RES));
  glProgramUniform1ui(m_programRenderGeometry, 6, PARTICLE_COUNT);
  glProgramUniform1f(m_programRenderGeometry, 7, pointRadius);
  glProgramUniform1i(m_programRenderGeometry, 8, m_options.colorMode);
  glBindVertexArray(m_swapFrame ? m_vao2 : m_vao1);
  const uint32_t index_count = 6 * PARTICLE_COUNT;
  glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

  // Step 7.1: Perform curvature flow (multiple iterations).
  glDisable(GL_DEPTH_TEST);
  glBindVertexArray(m_vao3);
  glUseProgram(m_programRenderCurvature);
  glProgramUniformMatrix4fv(m_programRenderCurvature, 0, 1, GL_FALSE, glm::value_ptr(vp));
  glProgramUniformMatrix4fv(m_programRenderCurvature, 2, 1, GL_FALSE, glm::value_ptr(projection));
  glProgramUniform2i(m_programRenderCurvature, 3, m_width, m_height);
  GLuint64 inputDepthTexHandle = m_texDepthHandle;
  bool swap = false;

  for (std::uint32_t i = 0; i < SMOOTH_ITERATIONS; ++i)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, swap ? m_fbo3 : m_fbo2);
    glClear(GL_COLOR_BUFFER_BIT);
    glProgramUniformHandleui64ARB(m_programRenderCurvature, 1, inputDepthTexHandle);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    inputDepthTexHandle = swap ? m_texTemp2Handle : m_texTemp1Handle;
    swap = !swap;
  }

  // Step 7.2: Do blinn-phong shading.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUseProgram(m_programRenderShading);
  glProgramUniformMatrix4fv(m_programRenderShading, 0, 1, GL_FALSE, glm::value_ptr(vp));
  glProgramUniformHandleui64ARB(m_programRenderShading, 1, inputDepthTexHandle);
  glProgramUniformHandleui64ARB(m_programRenderShading, 2, m_texColorHandle);
  glProgramUniform1ui(m_programRenderShading, 3, m_width);
  glProgramUniform1ui(m_programRenderShading, 4, m_height);
  glProgramUniformMatrix4fv(m_programRenderShading, 5, 1, GL_FALSE, glm::value_ptr(invProjection));
  glProgramUniformMatrix4fv(m_programRenderShading, 6, 1, GL_FALSE, glm::value_ptr(view));
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
  glEnable(GL_DEPTH_TEST);

  m_queries->endQuery();

  m_queries->readFinishedQueries(m_time);

  m_queries->incFrame();
}

void Simulation::resize(std::uint32_t width, std::uint32_t height)
{
  m_newWidth = width;
  m_newHeight = height;
}

Simulation::SimulationOptions& Simulation::options()
{
  return m_options;
}

const Simulation::SimulationTimes& Simulation::times() const
{
  return m_time;
}

void flut::Simulation::setIntegrationsPerFrame(std::uint32_t ipF)
{
  m_integrationsPerFrame = ipF;
}
