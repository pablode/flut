#include "Simulation.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <limits>
#include <cmath>

using namespace flut;
using namespace flut::core;

Simulation::Simulation(std::uint32_t width, std::uint32_t height)
  : SimulationBase()
  , width_(width)
  , height_(height)
  , newWidth_(width)
  , newHeight_(height)
  , swapFrame_{false}
  , frame_{0}
  , ipF_{1}
{
  // Shaders
  std::vector<char> compSource;
  loadFileText(RESOURCES_DIR "/buildGrid1.comp", compSource);
  programBuildGrid1_ = createComputeShader(compSource);
  loadFileText(RESOURCES_DIR "/buildGrid2.comp", compSource);
  programBuildGrid2_ = createComputeShader(compSource);
  loadFileText(RESOURCES_DIR "/buildGrid3.comp", compSource);
  programBuildGrid3_ = createComputeShader(compSource);
  loadFileText(RESOURCES_DIR "/simStep1.comp", compSource);
  programSimStep1_ = createComputeShader(compSource);
  loadFileText(RESOURCES_DIR "/simStep2.comp", compSource);
  programSimStep2_ = createComputeShader(compSource);

  std::vector<char> vertSource;
  std::vector<char> fragSource;
  loadFileText(RESOURCES_DIR "/renderGeometry.vert", vertSource);
  loadFileText(RESOURCES_DIR "/renderGeometry.frag", fragSource);
  programRenderGeometry_ = createVertFragShader(vertSource, fragSource);
  loadFileText(RESOURCES_DIR "/renderFlat.frag", fragSource);
  programRenderFlat_ = createVertFragShader(vertSource, fragSource);
  loadFileText(RESOURCES_DIR "/renderBoundingBox.vert", vertSource);
  loadFileText(RESOURCES_DIR "/renderCurvature.frag", fragSource);
  programRenderCurvature_ = createVertFragShader(vertSource, fragSource);
  loadFileText(RESOURCES_DIR "/renderShading.frag", fragSource);
  programRenderShading_ = createVertFragShader(vertSource, fragSource);

  // FBOs and Textures
  texDepth_ = createDepthTexture(width, height);
  texDepthHandle_ = makeTextureResident(texDepth_);
  texColor_ = createRGB32FColorTexture(width, height);
  texColorHandle_ = makeTextureResident(texColor_);
  texTemp1_ = createR32FColorTexture(width, height);
  texTemp1Handle_ = makeTextureResident(texTemp1_);
  texTemp2_ = createR32FColorTexture(width, height);
  texTemp2Handle_ = makeTextureResident(texTemp2_);
  fbo1_ = createFullFBO(texDepth_, {texColor_});
  fbo2_ = createFlatFBO(texTemp1_);
  fbo3_ = createFlatFBO(texTemp2_);

  // Precalc weight functions
  weightConstViscosity_ = static_cast<float>(45.0f / (M_PI * std::pow(KERNEL_RADIUS, 6)));
  weightConstPressure_ = static_cast<float>(45.0f / (M_PI * std::pow(KERNEL_RADIUS, 6)));
  weightConstKernel_ = static_cast<float>(315.0f / (64.0f * M_PI * std::pow(KERNEL_RADIUS, 9)));

  // Bounding Box
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

  std::vector<float> bboxVertexData;
  bboxVertexData.reserve(bboxVertices.size() * 3);
  for (const auto& vertex : bboxVertices)
  {
    bboxVertexData.push_back(vertex.x);
    bboxVertexData.push_back(vertex.y);
    bboxVertexData.push_back(vertex.z);
  }
  bufBBoxVertices_ = createBuffer(bboxVertexData, false);

  const std::vector<std::uint32_t> bboxIndices {
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4,
    4, 5, 1, 1, 0, 4,
    3, 2, 6, 6, 7, 3
  };

  bufBBoxIndices_ = createBuffer(bboxIndices, false);
  vao3_ = createBBoxVAO(bufBBoxVertices_, bufBBoxIndices_);

  // Uniform Grid
  glCreateTextures(GL_TEXTURE_3D, 1, &texGrid_);
  glTextureStorage3D(texGrid_, 1, GL_R32UI, GRID_RES.x, GRID_RES.y, GRID_RES.z);
  texGridImgHandle_ = glGetImageHandleARB(texGrid_, 0, GL_FALSE, 0, GL_R32UI);
  glMakeImageHandleResidentARB(texGridImgHandle_, GL_READ_WRITE);

  glCreateBuffers(1, &bufCounters_);
  glNamedBufferStorage(bufCounters_, 4, nullptr, GL_DYNAMIC_STORAGE_BIT);

  // Buffers
  preset1();

  // Other
  glCreateQueries(GL_TIME_ELAPSED, 4, &timerQueries_[0][0]);
  glCreateQueries(GL_TIME_ELAPSED, 4, &timerQueries_[1][0]);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

Simulation::~Simulation()
{
  glDeleteQueries(6, &timerQueries_[0][0]);
  glDeleteQueries(6, &timerQueries_[1][0]);
  deleteFBO(fbo1_);
  deleteFBO(fbo2_);
  deleteFBO(fbo3_);
  makeTextureNonResident(texDepthHandle_);
  deleteTexture(texDepth_);
  makeTextureNonResident(texColorHandle_);
  deleteTexture(texColor_);
  makeTextureNonResident(texTemp1Handle_);
  deleteTexture(texTemp1_);
  makeTextureNonResident(texTemp2Handle_);
  deleteTexture(texTemp2_);
  deleteShader(programBuildGrid1_);
  deleteShader(programBuildGrid2_);
  deleteShader(programBuildGrid3_);
  deleteShader(programSimStep1_);
  deleteShader(programSimStep2_);
  deleteShader(programRenderFlat_);
  deleteShader(programRenderGeometry_);
  deleteShader(programRenderCurvature_);
  deleteShader(programRenderShading_);
  deleteBuffer(bufBBoxVertices_);
  deleteBuffer(bufBBoxIndices_);
  deleteBuffer(bufParticles1_);
  deleteBuffer(bufParticles2_);
  makeTextureNonResident(texGridImgHandle_);
  deleteTexture(texGrid_);
  deleteBuffer(bufCounters_);
  deleteVAO(vao1_);
  deleteVAO(vao2_);
  deleteVAO(vao3_);
}

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

void Simulation::preset1()
{
  if (bufParticles1_)
    deleteBuffer(bufParticles1_);
  if (bufParticles2_)
    deleteBuffer(bufParticles2_);
  if (vao1_)
    deleteVAO(vao1_);
  if (vao2_)
    deleteVAO(vao2_);

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

  glCreateBuffers(1, &bufParticles1_);
  glCreateBuffers(1, &bufParticles2_);

  const auto size = PARTICLE_COUNT * sizeof(Particle);
  glNamedBufferStorage(bufParticles1_, size, particles.data(), 0);
  glNamedBufferStorage(bufParticles2_, size, particles.data(), 0);

  vao1_ = createParticleVAO(bufParticles1_);
  vao2_ = createParticleVAO(bufParticles2_);
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
    deleteFBO(fbo1_);
    deleteFBO(fbo2_);
    deleteFBO(fbo3_);
    makeTextureNonResident(texDepthHandle_);
    deleteTexture(texDepth_);
    makeTextureNonResident(texColorHandle_);
    deleteTexture(texColor_);
    makeTextureNonResident(texTemp1Handle_);
    deleteTexture(texTemp1_);
    makeTextureNonResident(texTemp2Handle_);
    deleteTexture(texTemp2_);
    texDepth_ = createDepthTexture(width_, height_);
    texDepthHandle_ = makeTextureResident(texDepth_);
    texColor_ = createRGB32FColorTexture(width_, height_);
    texColorHandle_ = makeTextureResident(texColor_);
    texTemp1_ = createR32FColorTexture(width_, height_);
    texTemp1Handle_ = makeTextureResident(texTemp1_);
    texTemp2_ = createR32FColorTexture(width_, height_);
    texTemp2Handle_ = makeTextureResident(texTemp2_);
    fbo1_ = createFullFBO(texDepth_, {texColor_});
    fbo2_ = createFlatFBO(texTemp1_);
    fbo3_ = createFlatFBO(texTemp2_);
  }

  const glm::vec3 invCellSize = glm::vec3(GRID_RES) * (1.0f - 0.001f) / GRID_SIZE;

  glViewport(0, 0, width_, height_);

  time_.gridBuildMs = 0.0f;
  time_.simStep1Ms = 0.0f;
  time_.simStep2Ms = 0.0f;
  time_.renderMs = 0.0f;

  if (frame_ > 1)
  {
    GLuint64 elapsedTime = 0;
    glGetQueryObjectui64v(lastQuery[3], GL_QUERY_RESULT, &elapsedTime);
    time_.renderMs = elapsedTime / 1000000.0f;
  }

  for (std::uint32_t f = 0; f < ipF_; f++)
  {
    if (frame_ > 1)
    {
      GLuint64 elapsedTime = 0;
      glGetQueryObjectui64v(lastQuery[0], GL_QUERY_RESULT, &elapsedTime);
      time_.gridBuildMs += elapsedTime / 1000000.0f;
      glGetQueryObjectui64v(lastQuery[1], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep1Ms += elapsedTime / 1000000.0f;
      glGetQueryObjectui64v(lastQuery[2], GL_QUERY_RESULT, &elapsedTime);
      time_.simStep2Ms += elapsedTime / 1000000.0f;
    }

    // 1. Build Voxel Grid
    glBeginQuery(GL_TIME_ELAPSED, query[0]);
    const std::uint32_t fClearValue = 0;
    glClearTexImage(texGrid_, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &fClearValue);
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    glUseProgram(programBuildGrid1_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles1_ : bufParticles2_);
    glProgramUniformHandleui64ARB(programBuildGrid1_, 0, texGridImgHandle_);
    glProgramUniform3fv(programBuildGrid1_, 1, 1, glm::value_ptr(invCellSize));
    glProgramUniform3fv(programBuildGrid1_, 2, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform1ui(programBuildGrid1_, 3, PARTICLE_COUNT);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32 * 32, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    const std::uint32_t uiClearValue = 0;
    glClearNamedBufferData(bufCounters_, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &uiClearValue);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glUseProgram(programBuildGrid2_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufCounters_);
    glProgramUniformHandleui64ARB(programBuildGrid2_, 0, texGridImgHandle_);
    glProgramUniform3iv(programBuildGrid2_, 1, 1, glm::value_ptr(GRID_RES));
    glDispatchCompute(
      (GRID_RES.x + 4 - 1) / 4 * 4,
      (GRID_RES.y + 4 - 1) / 4 * 4,
      (GRID_RES.z + 4 - 1) / 4 * 4
    );
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glUseProgram(programBuildGrid3_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles1_ : bufParticles2_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, swapFrame_ ? bufParticles2_ : bufParticles1_);
    glProgramUniformHandleui64ARB(programBuildGrid3_, 0, texGridImgHandle_);
    glProgramUniform3fv(programBuildGrid3_, 1, 1, glm::value_ptr(invCellSize));
    glProgramUniform3fv(programBuildGrid3_, 2, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform1ui(programBuildGrid3_, 3, PARTICLE_COUNT);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32 * 32, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    // 2. Simulation Step 1: Compute particle density and pressure
    glBeginQuery(GL_TIME_ELAPSED, query[1]);
    glUseProgram(programSimStep1_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles2_ : bufParticles1_);
    glProgramUniformHandleui64ARB(programSimStep1_, 0, texGridImgHandle_);
    glProgramUniform3fv(programSimStep1_, 1, 1, glm::value_ptr(invCellSize));
    glProgramUniform3fv(programSimStep1_, 2, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform3iv(programSimStep1_, 3, 1, glm::value_ptr(GRID_RES));
    glProgramUniform1ui(programSimStep1_, 4, PARTICLE_COUNT);
    glProgramUniform1f(programSimStep1_, 5, MASS);
    glProgramUniform1f(programSimStep1_, 6, KERNEL_RADIUS);
    glProgramUniform1f(programSimStep1_, 7, weightConstKernel_);
    glProgramUniform1f(programSimStep1_, 8, STIFFNESS);
    glProgramUniform1f(programSimStep1_, 9, REST_DENSITY);
    glProgramUniform1f(programSimStep1_, 10, REST_PRESSURE);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32 * 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    // 3. Simulation Step 2: Update force, integrate velocity and position, handle boundaries
    glBeginQuery(GL_TIME_ELAPSED, query[2]);
    glUseProgram(programSimStep2_);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufParticles2_ : bufParticles1_);
    glProgramUniformHandleui64ARB(programSimStep2_, 0, texGridImgHandle_);
    glProgramUniform3fv(programSimStep2_, 1, 1, glm::value_ptr(invCellSize));
    glProgramUniform1f(programSimStep2_, 2, DT * options_.deltaTimeMod);
    glProgramUniform3fv(programSimStep2_, 3, 1, glm::value_ptr(GRID_SIZE));
    glProgramUniform3fv(programSimStep2_, 4, 1, glm::value_ptr(GRID_ORIGIN));
    glProgramUniform1ui(programSimStep2_, 5, PARTICLE_COUNT);
    glProgramUniform3iv(programSimStep2_, 6, 1, glm::value_ptr(GRID_RES));
    glProgramUniform3fv(programSimStep2_, 7, 1, &options_.gravity[0]);
    glProgramUniform1f(programSimStep2_, 8, MASS);
    glProgramUniform1f(programSimStep2_, 9, KERNEL_RADIUS);
    glProgramUniform1f(programSimStep2_, 10, VIS_COEFF);
    glProgramUniform1f(programSimStep2_, 11, weightConstViscosity_);
    glProgramUniform1f(programSimStep2_, 12, weightConstPressure_);
    glDispatchCompute((PARTICLE_COUNT + 32 - 1) / 32 * 32, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    swapFrame_ = !swapFrame_;

    lastQuery = timerQueries_[swapFrame_ ? 0 : 1];
    query = timerQueries_[swapFrame_ ? 1 : 0];
  }

  // 4.1 Geometry Pass (or flat)
  GLuint renderProgram;
  glBeginQuery(GL_TIME_ELAPSED, query[3]);
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
  const float pointRadius = options_.shadingMode ? PARTICLE_RADIUS * 7.5f : PARTICLE_RADIUS * 3.0f;
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
    // 4.2 Curvature Flow
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
      glDrawElements(GL_TRIANGLES, 32, GL_UNSIGNED_INT, nullptr);
      inputDepthTexHandle = swap ? texTemp2Handle_ : texTemp1Handle_;
      swap = !swap;
    }

    // 4.3 Shading Pass
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

    glDrawElements(GL_TRIANGLES, 32, GL_UNSIGNED_INT, nullptr);
    glEnable(GL_DEPTH_TEST);
  }
  glEndQuery(GL_TIME_ELAPSED);
}

GLuint Simulation::createParticleVAO(GLuint ssbo) const
{
  GLuint handle;
  glCreateVertexArrays(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create VAO.");
  }

  glEnableVertexArrayAttrib(handle, 0);
  glVertexArrayVertexBuffer(handle, 0, ssbo, 0, sizeof(Particle));
  glVertexArrayAttribBinding(handle, 0, 0);
  glVertexArrayAttribFormat(handle, 0, 3, GL_FLOAT, GL_FALSE, 0);
  return handle;
}

GLuint Simulation::createBBoxVAO(GLuint vertices, GLuint indices) const
{
  GLuint handle;
  glCreateVertexArrays(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create VAO.");
  }

  glEnableVertexArrayAttrib(handle, 0);
  glVertexArrayVertexBuffer(handle, 0, vertices, 0, 3 * sizeof(float));
  glVertexArrayAttribBinding(handle, 0, 0);
  glVertexArrayAttribFormat(handle, 0, 3, GL_FLOAT, GL_FALSE, 0);

  glEnableVertexArrayAttrib(handle, 1);
  glVertexArrayElementBuffer(handle, indices);
  glVertexArrayAttribBinding(handle, 1, 0);
  glVertexArrayAttribFormat(handle, 1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
  return handle;
}

void Simulation::deleteVAO(GLuint handle)
{
  glDeleteVertexArrays(1, &handle);
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
  ipF_ = ipF;
}

void Simulation::loadFileText(const std::string& filePath, std::vector<char>& text) const
{
  std::ifstream file{filePath, std::ios_base::in | std::ios_base::binary};
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open file: " + filePath);
  }
  file.seekg(0, std::ios_base::end);
  text.resize(file.tellg());
  file.seekg(0, std::ios_base::beg);
  file.read(text.data(), text.size());
}
