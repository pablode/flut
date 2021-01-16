#include "Simulation.hpp"
#include "GlHelper.hpp"

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
  : width_(width)
  , height_(height)
  , newWidth_(width)
  , newHeight_(height)
  , swapFrame_{false}
  , frame_{0}
  , integrationsPerFrame_{1}
{
#ifndef NDEBUG
  GlHelper::enableDebugHooks();
#endif

  // Shaders
  programSimStep1_ = GlHelper::createComputeShader(RESOURCES_DIR "/simStep1.comp");
  programSimStep2_ = GlHelper::createComputeShader(RESOURCES_DIR "/simStep2.comp");
  programSimStep3_ = GlHelper::createComputeShader(RESOURCES_DIR "/simStep3.comp");
  programSimStep4_ = GlHelper::createComputeShader(RESOURCES_DIR "/simStep4.comp");
  programSimStep5_ = GlHelper::createComputeShader(RESOURCES_DIR "/simStep5.comp");
  programSimStep6_ = GlHelper::createComputeShader(RESOURCES_DIR "/simStep6.comp");

  programRenderGeometry_ = GlHelper::createVertFragShader(RESOURCES_DIR "/renderGeometry.vert", RESOURCES_DIR "/renderGeometry.frag");
  programRenderFlat_ = GlHelper::createVertFragShader(RESOURCES_DIR "/renderGeometry.vert", RESOURCES_DIR "/renderFlat.frag");
  programRenderCurvature_ = GlHelper::createVertFragShader(RESOURCES_DIR "/renderBoundingBox.vert", RESOURCES_DIR "/renderCurvature.frag");
  programRenderShading_ = GlHelper::createVertFragShader(RESOURCES_DIR "/renderBoundingBox.vert", RESOURCES_DIR "/renderShading.frag");

  // Precalc weight functions
  weightConstViscosity_ = static_cast<float>(45.0f / (M_PI * std::pow(KERNEL_RADIUS, 6)));
  weightConstPressure_ = static_cast<float>(45.0f / (M_PI * std::pow(KERNEL_RADIUS, 6)));
  weightConstKernel_ = static_cast<float>(315.0f / (64.0f * M_PI * std::pow(KERNEL_RADIUS, 9)));

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
  glCreateBuffers(1, &bufBBoxVertices_);
  glNamedBufferStorage(bufBBoxVertices_, bboxVertices.size() * sizeof(float) * 3, glm::value_ptr(bboxVertices.data()[0]), 0);

  const std::vector<std::uint32_t> bboxIndices {
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4,
    4, 5, 1, 1, 0, 4,
    3, 2, 6, 6, 7, 3
  };
  glCreateBuffers(1, &bufBBoxIndices_);
  glNamedBufferStorage(bufBBoxIndices_, bboxIndices.size() * sizeof(std::uint32_t), bboxIndices.data(), 0);

  glCreateVertexArrays(1, &vao3_);
  glEnableVertexArrayAttrib(vao3_, 0);
  glVertexArrayVertexBuffer(vao3_, 0, bufBBoxVertices_, 0, 3 * sizeof(float));
  glVertexArrayAttribBinding(vao3_, 0, 0);
  glVertexArrayAttribFormat(vao3_, 0, 3, GL_FLOAT, GL_FALSE, 0);
  glEnableVertexArrayAttrib(vao3_, 1);
  glVertexArrayElementBuffer(vao3_, bufBBoxIndices_);
  glVertexArrayAttribBinding(vao3_, 1, 0);
  glVertexArrayAttribFormat(vao3_, 1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float));

  // Uniform grid
  glCreateTextures(GL_TEXTURE_3D, 1, &texGrid_);
  glTextureStorage3D(texGrid_, 1, GL_R32UI, GRID_RES.x, GRID_RES.y, GRID_RES.z);
  texGridImgHandle_ = glGetImageHandleARB(texGrid_, 0, GL_FALSE, 0, GL_R32UI);
  glMakeImageHandleResidentARB(texGridImgHandle_, GL_READ_WRITE);

  glCreateBuffers(1, &bufCounters_);
  glNamedBufferStorage(bufCounters_, 4, nullptr, GL_DYNAMIC_STORAGE_BIT);

  // Velocity texture
  glCreateTextures(GL_TEXTURE_3D, 1, &texVelocity_);
  glTextureStorage3D(texVelocity_, 1, GL_RGBA32F, GRID_RES.x, GRID_RES.y, GRID_RES.z);
  texVelocityHandle_ = glGetTextureHandleARB(texVelocity_);
  glMakeTextureHandleResidentARB(texVelocityHandle_);
  texVelocityImgHandle_ = glGetImageHandleARB(texVelocity_, 0, GL_FALSE, 0, GL_RGBA32F);
  glMakeImageHandleResidentARB(texVelocityImgHandle_, GL_READ_WRITE);

  glTextureParameteri(texVelocity_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texVelocity_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
  glCreateBuffers(1, &bufParticles1_);
  glCreateBuffers(1, &bufParticles2_);
  glNamedBufferStorage(bufParticles1_, size, particles.data(), 0);
  glNamedBufferStorage(bufParticles2_, size, particles.data(), 0);

  glCreateVertexArrays(1, &vao1_);
  glEnableVertexArrayAttrib(vao1_, 0);
  glVertexArrayVertexBuffer(vao1_, 0, bufParticles1_, 0, sizeof(Particle));
  glVertexArrayAttribBinding(vao1_, 0, 0);
  glVertexArrayAttribFormat(vao1_, 0, 3, GL_FLOAT, GL_FALSE, 0);

  glCreateVertexArrays(1, &vao2_);
  glEnableVertexArrayAttrib(vao2_, 0);
  glVertexArrayVertexBuffer(vao2_, 0, bufParticles2_, 0, sizeof(Particle));
  glVertexArrayAttribBinding(vao2_, 0, 0);
  glVertexArrayAttribFormat(vao2_, 0, 3, GL_FLOAT, GL_FALSE, 0);

  // Timer queries
  glCreateQueries(GL_TIME_ELAPSED, 7, &timerQueries_[0][0]);
  glCreateQueries(GL_TIME_ELAPSED, 7, &timerQueries_[1][0]);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

  // Default state
  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);

  // Textures and buffers
  createFrameObjects();
}

void flut::Simulation::createFrameObjects()
{
  glCreateTextures(GL_TEXTURE_2D, 1, &texDepth_);
  glTextureStorage2D(texDepth_, 1, GL_DEPTH_COMPONENT24, width_, height_);
  glTextureParameteri(texDepth_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texDepth_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texDepthHandle_ = glGetTextureHandleARB(texDepth_);
  glMakeTextureHandleResidentARB(texDepthHandle_);

  glCreateTextures(GL_TEXTURE_2D, 1, &texColor_);
  glTextureStorage2D(texColor_, 1, GL_RGB32F, width_, height_);
  glTextureParameteri(texColor_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texColor_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texColorHandle_ = glGetTextureHandleARB(texColor_);
  glMakeTextureHandleResidentARB(texColorHandle_);

  glCreateTextures(GL_TEXTURE_2D, 1, &texTemp1_);
  glTextureStorage2D(texTemp1_, 1, GL_R32F, width_, height_);
  glTextureParameteri(texTemp1_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texTemp1_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texTemp1Handle_ = glGetTextureHandleARB(texTemp1_);
  glMakeTextureHandleResidentARB(texTemp1Handle_);

  glCreateTextures(GL_TEXTURE_2D, 1, &texTemp2_);
  glTextureStorage2D(texTemp2_, 1, GL_R32F, width_, height_);
  glTextureParameteri(texTemp2_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(texTemp2_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  texTemp2Handle_ = glGetTextureHandleARB(texTemp2_);
  glMakeTextureHandleResidentARB(texTemp2Handle_);

  glCreateFramebuffers(1, &fbo1_);
  glNamedFramebufferTexture(fbo1_, GL_DEPTH_ATTACHMENT, texDepth_, 0);
  glNamedFramebufferTexture(fbo1_, GL_COLOR_ATTACHMENT0, texColor_, 0);

  glCreateFramebuffers(1, &fbo2_);
  glNamedFramebufferTexture(fbo2_, GL_COLOR_ATTACHMENT0, texTemp1_, 0);

  glCreateFramebuffers(1, &fbo3_);
  glNamedFramebufferTexture(fbo3_, GL_COLOR_ATTACHMENT0, texTemp2_, 0);
}

void flut::Simulation::deleteFrameObjects()
{
  glDeleteFramebuffers(1, &fbo1_);
  glDeleteFramebuffers(1, &fbo2_);
  glDeleteFramebuffers(1, &fbo3_);

  glMakeTextureHandleNonResidentARB(texDepthHandle_);
  glDeleteTextures(1, &texDepth_);

  glMakeTextureHandleNonResidentARB(texColorHandle_);
  glDeleteTextures(1, &texColor_);

  glMakeTextureHandleNonResidentARB(texTemp1Handle_);
  glDeleteTextures(1, &texTemp1_);

  glMakeTextureHandleNonResidentARB(texTemp2Handle_);
  glDeleteTextures(1, &texTemp2_);
}

Simulation::~Simulation()
{
  deleteFrameObjects();
  glDeleteProgram(programSimStep1_);
  glDeleteProgram(programSimStep2_);
  glDeleteProgram(programSimStep3_);
  glDeleteProgram(programSimStep5_);
  glDeleteProgram(programSimStep6_);
  glDeleteProgram(programRenderFlat_);
  glDeleteProgram(programRenderGeometry_);
  glDeleteProgram(programRenderCurvature_);
  glDeleteProgram(programRenderShading_);
  glDeleteBuffers(1, &bufBBoxVertices_);
  glDeleteBuffers(1, &bufBBoxIndices_);
  glDeleteBuffers(1, &bufParticles1_);
  glDeleteBuffers(1, &bufParticles2_);
  glMakeImageHandleNonResidentARB(texGridImgHandle_);
  glDeleteTextures(1, &texGrid_);
  glMakeImageHandleNonResidentARB(texVelocityImgHandle_);
  glMakeTextureHandleNonResidentARB(texVelocityHandle_);
  glDeleteTextures(1, &texVelocity_);
  glDeleteBuffers(1, &bufCounters_);
  glDeleteVertexArrays(1, &vao1_);
  glDeleteVertexArrays(1, &vao2_);
  glDeleteVertexArrays(1, &vao3_);
  glDeleteQueries(6, &timerQueries_[0][0]);
  glDeleteQueries(6, &timerQueries_[1][0]);
}

void Simulation::render(const Camera& camera, float dt)
{
  ++frame_;

  GLuint* lastQuery = timerQueries_[swapFrame_ ? 0 : 1];
  GLuint* query = timerQueries_[swapFrame_ ? 1 : 0];

  // Resize window if needed.
  if (width_ != newWidth_ || height_ != newHeight_)
  {
    width_ = newWidth_;
    height_ = newHeight_;
    deleteFrameObjects();
    createFrameObjects();
  }

  const glm::vec3 invCellSize = glm::vec3(GRID_RES) * (1.0f - 0.001f) / GRID_SIZE;

  glViewport(0, 0, width_, height_);

  time_.simStep1Ms = 0.0f;
  time_.simStep2Ms = 0.0f;
  time_.simStep3Ms = 0.0f;
  time_.simStep4Ms = 0.0f;
  time_.simStep5Ms = 0.0f;
  time_.simStep6Ms = 0.0f;
  time_.renderMs = 0.0f;

  if (frame_ > 1)
  {
    GLuint64 elapsedTime = 0;
    glGetQueryObjectui64v(lastQuery[6], GL_QUERY_RESULT, &elapsedTime);
    time_.renderMs = elapsedTime / 1000000.0f;
  }

  for (std::uint32_t f = 0; f < integrationsPerFrame_; f++)
  {
    if (frame_ > 1)
    {
      GLuint64 elapsedTime = 0;
      glGetQueryObjectui64v(lastQuery[0], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep1Ms += elapsedTime / 1000000.0f;
      glGetQueryObjectui64v(lastQuery[1], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep2Ms += elapsedTime / 1000000.0f;
      glGetQueryObjectui64v(lastQuery[2], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep3Ms += elapsedTime / 1000000.0f;
      glGetQueryObjectui64v(lastQuery[3], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep4Ms += elapsedTime / 1000000.0f;
      glGetQueryObjectui64v(lastQuery[4], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep5Ms += elapsedTime / 1000000.0f;
      glGetQueryObjectui64v(lastQuery[5], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep6Ms += elapsedTime / 1000000.0f;
    }

    // Step 1: Integrate position, do boundary handling.
    //         Write particle count to voxel grid.
    glBeginQuery(GL_TIME_ELAPSED, query[0]);
    const std::uint32_t fClearValue = 0;
    glClearTexImage(texGrid_, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &fClearValue);
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    glUseProgram(programSimStep1_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles1_ : bufParticles2_);
    glProgramUniformHandleui64ARB(programSimStep1_, 0, texGridImgHandle_);
    glProgramUniform3fv(programSimStep1_, 1, 1, glm::value_ptr(invCellSize));
    glProgramUniform3fv(programSimStep1_, 2, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform1ui(programSimStep1_, 3, PARTICLE_COUNT);
    glProgramUniform3fv(programSimStep1_, 4, 1, glm::value_ptr(GRID_SIZE));
    glProgramUniform1f(programSimStep1_, 5, DT * options_.deltaTimeMod);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    // Step 2: Write global particle array offsets into voxel grid.
    glBeginQuery(GL_TIME_ELAPSED, query[1]);
    const std::uint32_t uiClearValue = 0;
    glClearNamedBufferData(bufCounters_, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &uiClearValue);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glUseProgram(programSimStep2_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufCounters_);
    glProgramUniformHandleui64ARB(programSimStep2_, 0, texGridImgHandle_);
    glProgramUniform3iv(programSimStep2_, 1, 1, glm::value_ptr(GRID_RES));
    glDispatchCompute(
      (GRID_RES.x + 4 - 1) / 4,
      (GRID_RES.y + 4 - 1) / 4,
      (GRID_RES.z + 4 - 1) / 4
    );
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    // Step 3: Write particles to new location in second particle buffer.
    //         Write particle count to voxel grid (again).
    glBeginQuery(GL_TIME_ELAPSED, query[2]);
    glUseProgram(programSimStep3_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles1_ : bufParticles2_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, swapFrame_ ? bufParticles2_ : bufParticles1_);
    glProgramUniformHandleui64ARB(programSimStep3_, 0, texGridImgHandle_);
    glProgramUniform3fv(programSimStep3_, 1, 1, glm::value_ptr(invCellSize));
    glProgramUniform3fv(programSimStep3_, 2, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform1ui(programSimStep3_, 3, PARTICLE_COUNT);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    // Step 4: Write average voxel velocities into second 3D-texture.
    glBeginQuery(GL_TIME_ELAPSED, query[3]);
    glUseProgram(programSimStep4_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles2_ : bufParticles1_);
    glProgramUniformHandleui64ARB(programSimStep4_, 0, texGridImgHandle_);
    glProgramUniformHandleui64ARB(programSimStep4_, 1, texVelocityImgHandle_);
    glProgramUniform3iv(programSimStep4_, 2, 1, glm::value_ptr(GRID_RES));
    glDispatchCompute(
      (GRID_RES.x + 4 - 1) / 4,
      (GRID_RES.y + 4 - 1) / 4,
      (GRID_RES.z + 4 - 1) / 4
    );
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    // Step 5: Compute density and pressure for each particle.
    glBeginQuery(GL_TIME_ELAPSED, query[4]);
    glUseProgram(programSimStep5_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles2_ : bufParticles1_);
    glProgramUniformHandleui64ARB(programSimStep5_, 0, texGridImgHandle_);
    glProgramUniform3fv(programSimStep5_, 1, 1, glm::value_ptr(invCellSize));
    glProgramUniform3fv(programSimStep5_, 2, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform3iv(programSimStep5_, 3, 1, glm::value_ptr(GRID_RES));
    glProgramUniform1ui(programSimStep5_, 4, PARTICLE_COUNT);
    glProgramUniform1f(programSimStep5_, 5, MASS);
    glProgramUniform1f(programSimStep5_, 6, KERNEL_RADIUS);
    glProgramUniform1f(programSimStep5_, 7, weightConstKernel_);
    glProgramUniform1f(programSimStep5_, 8, STIFFNESS);
    glProgramUniform1f(programSimStep5_, 9, REST_DENSITY);
    glProgramUniform1f(programSimStep5_, 10, REST_PRESSURE);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    // Step 6: Compute pressure and viscosity forces, use them to write new velocity.
    //         For the old velocity, we use the coarse 3d-texture and do trilinear HW filtering.
    glBeginQuery(GL_TIME_ELAPSED, query[5]);
    glUseProgram(programSimStep6_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles2_ : bufParticles1_);
    glProgramUniformHandleui64ARB(programSimStep6_, 0, texGridImgHandle_);
    glProgramUniformHandleui64ARB(programSimStep6_, 1, texVelocityHandle_);
    glProgramUniform3fv(programSimStep6_, 2, 1, glm::value_ptr(invCellSize));
    glProgramUniform1f(programSimStep6_, 3, DT * options_.deltaTimeMod);
    glProgramUniform3fv(programSimStep6_, 4, 1, glm::value_ptr(GRID_SIZE));
    glProgramUniform3fv(programSimStep6_, 5, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform1ui(programSimStep6_, 6, PARTICLE_COUNT);
    glProgramUniform3iv(programSimStep6_, 7, 1, glm::value_ptr(GRID_RES));
    glProgramUniform3fv(programSimStep6_, 8, 1, &options_.gravity[0]);
    glProgramUniform1f(programSimStep6_, 9, MASS);
    glProgramUniform1f(programSimStep6_, 10, KERNEL_RADIUS);
    glProgramUniform1f(programSimStep6_, 11, VIS_COEFF);
    glProgramUniform1f(programSimStep6_, 12, weightConstViscosity_);
    glProgramUniform1f(programSimStep6_, 13, weightConstPressure_);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    swapFrame_ = !swapFrame_;

    lastQuery = timerQueries_[swapFrame_ ? 0 : 1];
    query = timerQueries_[swapFrame_ ? 1 : 0];
  }

  // Step 7: Render the geometry (points or screen-space spheres).
  GLuint renderProgram;
  glBeginQuery(GL_TIME_ELAPSED, query[6]);
  if (options_.shadingMode == 0) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    renderProgram = programRenderFlat_;
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1_);
    renderProgram = programRenderGeometry_;
  }
  glUseProgram(renderProgram);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  const float pointRadius = options_.shadingMode ? PARTICLE_RADIUS * 6.0f : PARTICLE_RADIUS * 3.5f;
  const float pointScale = 650.0f;
  const auto& view = camera.view();
  const auto& projection = camera.projection();
  const auto& invProjection = camera.invProjection();
  const glm::mat4 mvp = projection * view;
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles2_ : bufParticles1_);
  glProgramUniformMatrix4fv(renderProgram, 0, 1, GL_FALSE, glm::value_ptr(mvp));
  glProgramUniformMatrix4fv(renderProgram, 1, 1, GL_FALSE, glm::value_ptr(view));
  glProgramUniformMatrix4fv(renderProgram, 2, 1, GL_FALSE, glm::value_ptr(projection));
  glProgramUniform3fv(renderProgram, 3, 1, glm::value_ptr(GRID_SIZE));
  glProgramUniform3fv(renderProgram, 4, 1, glm::value_ptr(GRID_ORIGIN));
  glProgramUniform3iv(renderProgram, 5, 1, glm::value_ptr(GRID_RES));
  glProgramUniform1ui(renderProgram, 6, PARTICLE_COUNT);
  glProgramUniform1f(renderProgram, 7, pointRadius);
  glProgramUniform1f(renderProgram, 8, pointScale);
  glProgramUniform1i(renderProgram, 9, options_.colorMode);
  glProgramUniform1i(renderProgram, 10, options_.shadingMode);
  glBindVertexArray(swapFrame_ ? vao2_ : vao1_);
  glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);

  if (options_.shadingMode == 1)
  {
    // Step 7.1: Perform curvature flow (multiple iterations).
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(vao3_);
    glUseProgram(programRenderCurvature_);
    glProgramUniformMatrix4fv(programRenderCurvature_, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glProgramUniformMatrix4fv(programRenderCurvature_, 2, 1, GL_FALSE, glm::value_ptr(projection));
    glProgramUniform2i(programRenderCurvature_, 3, width_, height_);
    GLuint64 inputDepthTexHandle = texDepthHandle_;
    bool swap = false;

    for (std::uint32_t i = 0; i < SMOOTH_ITERATIONS; ++i)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, swap ? fbo3_ : fbo2_);
      glClear(GL_COLOR_BUFFER_BIT);
      glProgramUniformHandleui64ARB(programRenderCurvature_, 1, inputDepthTexHandle);
      glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
      inputDepthTexHandle = swap ? texTemp2Handle_ : texTemp1Handle_;
      swap = !swap;
    }

    // Step 7.2: Do blinn-phong shading.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programRenderShading_);
    glProgramUniformMatrix4fv(programRenderShading_, 0, 1, GL_FALSE, glm::value_ptr(mvp));
    glProgramUniformHandleui64ARB(programRenderShading_, 1, inputDepthTexHandle);
    glProgramUniformHandleui64ARB(programRenderShading_, 2, texColorHandle_);
    glProgramUniform1ui(programRenderShading_, 3, width_);
    glProgramUniform1ui(programRenderShading_, 4, height_);
    glProgramUniformMatrix4fv(programRenderShading_, 5, 1, GL_FALSE, glm::value_ptr(invProjection));
    glProgramUniformMatrix4fv(programRenderShading_, 6, 1, GL_FALSE, glm::value_ptr(view));

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
    glEnable(GL_DEPTH_TEST);
  }
  glEndQuery(GL_TIME_ELAPSED);
}

void Simulation::resize(std::uint32_t width, std::uint32_t height)
{
  newWidth_ = width;
  newHeight_ = height;
}

Simulation::SimulationOptions& Simulation::options()
{
  return options_;
}

const Simulation::SimulationTimes& Simulation::times() const
{
  return time_;
}

void flut::Simulation::setIntegrationsPerFrame(std::uint32_t ipF)
{
  integrationsPerFrame_ = ipF;
}
