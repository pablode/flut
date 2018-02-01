#include "Simulation.hpp"

#define _USE_MATH_DEFINES
#include <cmath>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <limits>

ansimproj::Simulation::Simulation(const std::uint32_t &width, const std::uint32_t &height)
  : BaseRenderer()
  , width_(width)
  , height_(height)
  , swapFrame_{false}
  , frame_{0}
  , bufColor_{0}
  , bufGridUnsorted_{0}
  , bufGridSorted_{0}
  , bufGridIndices_{0}
  , bufPosition1_{0}
  , bufVelocity1_{0}
  , bufPosition2_{0}
  , bufVelocity2_{0}
  , bufDensity_{0}
  , vao1_{0}
  , vao2_{0} {

  // Shaders
  auto shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridInsert.comp");
  programGridInsert_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridSort.comp");
  programGridSort_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridIndexing.comp");
  programGridIndexing_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/densityComputation.comp");
  programDensityComputation_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/forceUpdate.comp");
  programForceUpdate_ = createComputeShader(shaderSource);
  auto vert = core::Utils::loadFileText(RESOURCES_PATH "/renderGeometry.vert");
  auto frag = core::Utils::loadFileText(RESOURCES_PATH "/renderGeometry.frag");
  programRenderGeometry_ = createVertFragShader(vert, frag);
  frag = core::Utils::loadFileText(RESOURCES_PATH "/renderFlat.frag");
  programRenderFlat_ = createVertFragShader(vert, frag);
  vert = core::Utils::loadFileText(RESOURCES_PATH "/renderBoundingBox.vert");
  frag = core::Utils::loadFileText(RESOURCES_PATH "/renderCurvature.frag");
  programRenderCurvature_ = createVertFragShader(vert, frag);
  frag = core::Utils::loadFileText(RESOURCES_PATH "/renderShading.frag");
  programRenderShading_ = createVertFragShader(vert, frag);

  // FBOs and Textures
  texDepth_ = createDepthTexture(width, height);
  texColor_ = createRGB32FColorTexture(width, height);
  texTemp1_ = createR32FColorTexture(width, height);
  texTemp2_ = createR32FColorTexture(width, height);
  fbo1_ = createFullFBO(texDepth_, {texColor_});
  fbo2_ = createFlatFBO(texTemp1_);
  fbo3_ = createFlatFBO(texTemp2_);

  // Precalc weight functions
  weightConstViscosity_ = static_cast<float>(45.0f / (M_PI * std::pow(RANGE, 6)));
  weightConstPressure_ = static_cast<float>(45.0f / (M_PI * std::pow(RANGE, 6)));
  weightConstDefault_ = static_cast<float>(315.0f / (64.0f * M_PI * std::pow(RANGE, 9)));

  // Bounding Box
  const std::vector<Eigen::Vector3f> bboxVertices {
    GRID_ORIGIN + Eigen::Vector3f{0.0f, 0.0f, GRID_LEN(2)},
    GRID_ORIGIN + Eigen::Vector3f{GRID_LEN(0), 0.0f, GRID_LEN(2)},
    GRID_ORIGIN + Eigen::Vector3f{GRID_LEN(0), GRID_LEN(1), GRID_LEN(2)},
    GRID_ORIGIN + Eigen::Vector3f{0.0f, GRID_LEN(1), GRID_LEN(2)},
    GRID_ORIGIN + Eigen::Vector3f{0.0f, 0.0f, 0.0f},
    GRID_ORIGIN + Eigen::Vector3f{GRID_LEN(0), 0.0f, 0.0f},
    GRID_ORIGIN + Eigen::Vector3f{GRID_LEN(0), GRID_LEN(1), 0.0f},
    GRID_ORIGIN + Eigen::Vector3f{0.0f, GRID_LEN(1), 0.0f},
  };
  std::vector<float> bboxVertexData;
  for (const auto& vertex : bboxVertices) {
    bboxVertexData.push_back(vertex(0));
    bboxVertexData.push_back(vertex(1));
    bboxVertexData.push_back(vertex(2));
  }
  bufBBoxVertices_ = createBuffer(bboxVertexData, false);
  // clang-format off
  const std::vector<std::uint32_t> bboxIndices {
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4,
    4, 5, 1, 1, 0, 4,
    3, 2, 6, 6, 7, 3
  };
  // clang-format on
  bufBBoxIndices_ = createBuffer(bboxIndices, false);

  // Buffers
  preset1();

  // Other
  vao3_ = createBBoxVAO(bufBBoxVertices_, bufBBoxIndices_);
  glGenQueries(6, &timerQueries_[0][0]);
  glGenQueries(6, &timerQueries_[1][0]);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

ansimproj::Simulation::~Simulation() {
  glDeleteQueries(6, &timerQueries_[0][0]);
  glDeleteQueries(6, &timerQueries_[1][0]);
  deleteFBO(fbo1_);
  deleteFBO(fbo2_);
  deleteFBO(fbo3_);
  deleteTexture(texDepth_);
  deleteTexture(texColor_);
  deleteTexture(texTemp1_);
  deleteTexture(texTemp2_);
  deleteShader(programGridInsert_);
  deleteShader(programGridSort_);
  deleteShader(programGridIndexing_);
  deleteShader(programDensityComputation_);
  deleteShader(programForceUpdate_);
  deleteShader(programRenderFlat_);
  deleteShader(programRenderGeometry_);
  deleteShader(programRenderCurvature_);
  deleteShader(programRenderShading_);
  deleteBuffer(bufBBoxVertices_);
  deleteBuffer(bufBBoxIndices_);
  deleteBuffer(bufColor_);
  deleteBuffer(bufGridUnsorted_);
  deleteBuffer(bufGridSorted_);
  deleteBuffer(bufGridIndices_);
  deleteBuffer(bufPosition1_);
  deleteBuffer(bufPosition2_);
  deleteBuffer(bufVelocity1_);
  deleteBuffer(bufVelocity2_);
  deleteBuffer(bufDensity_);
  deleteVAO(vao1_);
  deleteVAO(vao2_);
  deleteVAO(vao3_);
}

void ansimproj::Simulation::preset1() {
  if (bufGridUnsorted_)
    deleteBuffer(bufGridUnsorted_);
  if (bufGridSorted_)
    deleteBuffer(bufGridSorted_);
  if (bufGridIndices_)
    deleteBuffer(bufGridIndices_);
  if (bufColor_)
    deleteBuffer(bufColor_);
  if (bufPosition1_)
    deleteBuffer(bufPosition1_);
  if (bufPosition2_)
    deleteBuffer(bufPosition2_);
  if (bufVelocity1_)
    deleteBuffer(bufVelocity1_);
  if (bufVelocity2_)
    deleteBuffer(bufVelocity2_);
  if (bufDensity_)
    deleteBuffer(bufDensity_);
  if (vao1_)
    deleteVAO(vao1_);
  if (vao2_)
    deleteVAO(vao2_);

  std::vector<GLuint> gridPairsData;
  gridPairsData.resize(PARTICLE_COUNT * 2);
  bufGridUnsorted_ = createBuffer(gridPairsData, true);
  bufGridSorted_ = createBuffer(gridPairsData, true);

  std::vector<GLuint> gridIndicesData;
  gridIndicesData.resize(GRID_VOXEL_COUNT * 2);
  bufGridIndices_ = createBuffer(gridIndicesData, true);

  std::vector<float> posData;
  std::vector<float> colData;
  posData.reserve(PARTICLE_COUNT * 3);
  colData.reserve(PARTICLE_COUNT * 3);
  for (std::uint32_t i = 0; i < PARTICLE_COUNT; ++i) {
    const float xi = ((std::rand() % 10000) / 10000.0f);
    const float yi = ((std::rand() % 10000) / 10000.0f);
    const float zi = ((std::rand() % 10000) / 10000.0f);
    const float x = xi * GRID_LEN(0);
    const float y = yi * GRID_LEN(1);
    const float z = zi * GRID_LEN(2);
    posData.push_back((GRID_ORIGIN(0) + x) / 2.0f);
    posData.push_back((GRID_ORIGIN(1) + y) / 2.0f);
    posData.push_back((GRID_ORIGIN(2) + z) / 2.0f);
    colData.push_back(0.0f);
    colData.push_back(0.0f);
    colData.push_back(1.0f);
  }
  bufPosition1_ = createBuffer(posData, true);
  bufPosition2_ = createBuffer(posData, true);
  bufColor_ = createBuffer(colData, false);

  std::vector<float> zeroFloatData;
  zeroFloatData.resize(PARTICLE_COUNT);
  bufDensity_ = createBuffer(zeroFloatData, true);

  zeroFloatData.resize(PARTICLE_COUNT * 3);
  bufVelocity1_ = createBuffer(zeroFloatData, true);
  bufVelocity2_ = createBuffer(zeroFloatData, true);

  vao1_ = createParticleVAO(bufPosition1_, bufColor_);
  vao2_ = createParticleVAO(bufPosition2_, bufColor_);
}

void ansimproj::Simulation::render(const ansimproj::core::Camera &camera, float dt) {
  constexpr auto localSize = 128;
  assert(PARTICLE_COUNT % localSize == 0);
  const std::uint32_t numWorkGroups = PARTICLE_COUNT / localSize;
  auto &lastQuery = timerQueries_[swapFrame_ ? 0 : 1];
  auto &query = timerQueries_[swapFrame_ ? 1 : 0];
  swapFrame_ = !swapFrame_;
  ++frame_;

  // 1.1 Generate Particle/Voxel mappings
  glBeginQuery(GL_TIME_ELAPSED, query[0]);
  glUseProgram(programGridInsert_);
  glProgramUniform3fv(programGridInsert_, 0, 1, GRID_LEN.data());
  glProgramUniform3fv(programGridInsert_, 1, 1, GRID_ORIGIN.data());
  glProgramUniform3uiv(programGridInsert_, 2, 1, GRID_RES.data());
  glProgramUniform1ui(programGridInsert_, 3, PARTICLE_COUNT);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridUnsorted_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufGridSorted_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glEndQuery(GL_TIME_ELAPSED);

  // 1.2 Sort Particle/Voxel mappings
  // TODO: replace bitonic mergesort with counting sort
  glBeginQuery(GL_TIME_ELAPSED, query[1]);
  glUseProgram(programGridSort_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufGridSorted_);
  constexpr auto N = PARTICLE_COUNT;
  for (std::uint32_t size = 2; size <= N; size *= 2) {
    for (std::uint32_t stride = size / 2; stride > 0; stride /= 2) {
      glProgramUniform1ui(programGridSort_, 0, size);
      glProgramUniform1ui(programGridSort_, 1, stride);
      glDispatchCompute(numWorkGroups, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
  }
  glEndQuery(GL_TIME_ELAPSED);

  // 1.3 Find Voxel indices and size
  const auto indexingWorkGroups = GRID_VOXEL_COUNT / localSize;
  assert(GRID_VOXEL_COUNT % localSize == 0);
  glBeginQuery(GL_TIME_ELAPSED, query[2]);
  glUseProgram(programGridIndexing_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufGridSorted_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridIndices_);
  glDispatchCompute(indexingWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glEndQuery(GL_TIME_ELAPSED);

  // 2. Density Computation
  glBeginQuery(GL_TIME_ELAPSED, query[3]);
  glUseProgram(programDensityComputation_);
  glProgramUniform3fv(programDensityComputation_, 0, 1, GRID_LEN.data());
  glProgramUniform3fv(programDensityComputation_, 1, 1, GRID_ORIGIN.data());
  glProgramUniform3uiv(programDensityComputation_, 2, 1, GRID_RES.data());
  glProgramUniform1f(programDensityComputation_, 3, MASS);
  glProgramUniform1f(programDensityComputation_, 4, RANGE);
  glProgramUniform1f(programDensityComputation_, 5, weightConstDefault_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridUnsorted_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufGridSorted_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufGridIndices_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bufDensity_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glEndQuery(GL_TIME_ELAPSED);

  // 3. Force Update
  glBeginQuery(GL_TIME_ELAPSED, query[4]);
  glUseProgram(programForceUpdate_);
  glProgramUniform1f(programForceUpdate_, 0, DT * options_.deltaTimeMod);
  glProgramUniform3fv(programForceUpdate_, 1, 1, GRID_LEN.data());
  glProgramUniform3fv(programForceUpdate_, 2, 1, GRID_ORIGIN.data());
  glProgramUniform3uiv(programForceUpdate_, 3, 1, GRID_RES.data());
  glProgramUniform3fv(programForceUpdate_, 4, 1, &options_.gravity[0]);
  glProgramUniform1f(programForceUpdate_, 5, K);
  glProgramUniform1f(programForceUpdate_, 6, MASS);
  glProgramUniform1f(programForceUpdate_, 7, RANGE);
  glProgramUniform1f(programForceUpdate_, 8, VIS_COEFF);
  glProgramUniform1f(programForceUpdate_, 9, REST_PRESSURE);
  glProgramUniform1f(programForceUpdate_, 10, REST_DENSITY);
  glProgramUniform1f(programForceUpdate_, 11, weightConstViscosity_);
  glProgramUniform1f(programForceUpdate_, 12, weightConstPressure_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridUnsorted_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufGridSorted_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufGridIndices_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bufDensity_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, swapFrame_ ? bufVelocity1_ : bufVelocity2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, swapFrame_ ? bufVelocity2_ : bufVelocity1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, swapFrame_ ? bufPosition2_ : bufPosition1_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glEndQuery(GL_TIME_ELAPSED);

  // 4.1 Geometry Pass (or flat)
  GLuint renderProgram;
  glBeginQuery(GL_TIME_ELAPSED, query[5]);
  if (options_.shadingMode == 0) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    renderProgram = programRenderFlat_;
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo1_);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    renderProgram = programRenderGeometry_;
  }
  glUseProgram(renderProgram);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  const float pointRadius = options_.shadingMode ? RANGE * 1.5f : RANGE / 2.0f;
  const float pointScale = 650.0f;
  const auto &view = camera.view();
  const auto &projection = camera.projection();
  const auto &invProjection = camera.invProjection();
  const Eigen::Matrix4f mvp = projection * view;
  glProgramUniformMatrix4fv(renderProgram, 0, 1, GL_FALSE, mvp.data());
  glProgramUniformMatrix4fv(renderProgram, 1, 1, GL_FALSE, view.data());
  glProgramUniformMatrix4fv(renderProgram, 2, 1, GL_FALSE, projection.data());
  glProgramUniform3fv(renderProgram, 3, 1, GRID_LEN.data());
  glProgramUniform3fv(renderProgram, 4, 1, GRID_ORIGIN.data());
  glProgramUniform3uiv(renderProgram, 5, 1, GRID_RES.data());
  glProgramUniform1ui(renderProgram, 6, PARTICLE_COUNT);
  glProgramUniform1f(renderProgram, 7, pointRadius);
  glProgramUniform1f(renderProgram, 8, pointScale);
  glProgramUniform1i(renderProgram, 9, options_.colorMode);
  glProgramUniform1i(renderProgram, 10, options_.shadingMode);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufPosition2_ : bufPosition1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufDensity_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, swapFrame_ ? bufVelocity2_ : bufVelocity1_);
  glBindVertexArray(swapFrame_ ? vao2_ : vao1_);
  glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
  if (options_.shadingMode == 1) {

    // 4.2 Curvature Flow
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(vao3_);
    glUseProgram(programRenderCurvature_);
    glProgramUniformMatrix4fv(programRenderCurvature_, 0, 1, GL_FALSE, mvp.data());
    glProgramUniform1i(programRenderCurvature_, 1, 0);
    glProgramUniformMatrix4fv(programRenderCurvature_, 2, 1, GL_FALSE, projection.data());
    glProgramUniform2i(programRenderCurvature_, 3, width_, height_);
    std::uint32_t inputDepthTex = texDepth_;
    bool swap = false;
    for (std::uint32_t i = 0; i < SMOOTH_ITERATIONS; ++i) {
      glBindFramebuffer(GL_FRAMEBUFFER, swap ? fbo3_ : fbo2_);
      glClear(GL_COLOR_BUFFER_BIT);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, inputDepthTex);
      glDrawElements(GL_TRIANGLES, 32, GL_UNSIGNED_INT, nullptr);
      inputDepthTex = swap ? texTemp2_ : texTemp1_;
      swap = !swap;
    }

    // 4.3 Shading Pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputDepthTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texColor_);
    glUseProgram(programRenderShading_);
    glProgramUniformMatrix4fv(programRenderShading_, 0, 1, GL_FALSE, mvp.data());
    glProgramUniform1i(programRenderShading_, 1, 0);
    glProgramUniform1i(programRenderShading_, 2, 1);
    glProgramUniform1ui(programRenderShading_, 3, width_);
    glProgramUniform1ui(programRenderShading_, 4, height_);
    glProgramUniformMatrix4fv(programRenderShading_, 5, 1, GL_FALSE, invProjection.data());
    glProgramUniformMatrix4fv(programRenderShading_, 6, 1, GL_FALSE, view.data());
    glDrawElements(GL_TRIANGLES, 32, GL_UNSIGNED_INT, nullptr);
    glEnable(GL_DEPTH_TEST);
  }
  glEndQuery(GL_TIME_ELAPSED);

  // Fetch GPU Timer Queries
  if (frame_ > 1) {
    GLuint64 elapsedTime = 0;
    glGetQueryObjectui64v(lastQuery[0], GL_QUERY_RESULT, &elapsedTime);
    time_.gridInsertMs = elapsedTime / 1000000.0f;
    glGetQueryObjectui64v(lastQuery[1], GL_QUERY_RESULT, &elapsedTime);
    time_.gridSortMs = elapsedTime / 1000000.0f;
    glGetQueryObjectui64v(lastQuery[2], GL_QUERY_RESULT, &elapsedTime);
    time_.gridIndexingMs = elapsedTime / 1000000.0f;
    glGetQueryObjectui64v(lastQuery[3], GL_QUERY_RESULT, &elapsedTime);
    time_.densityComputationMs = elapsedTime / 1000000.0f;
    glGetQueryObjectui64v(lastQuery[4], GL_QUERY_RESULT, &elapsedTime);
    time_.forceUpdateMs = elapsedTime / 1000000.0f;
    glGetQueryObjectui64v(lastQuery[5], GL_QUERY_RESULT, &elapsedTime);
    time_.rendering = elapsedTime / 1000000.0f;
  }
}

GLuint ansimproj::Simulation::createParticleVAO(const GLuint &vboPos, const GLuint &vboCol) const {
  GLuint handle;
  glGenVertexArrays(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create VAO.");
  }
  glBindVertexArray(handle);

  glBindBuffer(GL_ARRAY_BUFFER, vboPos);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, vboCol);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
  return handle;
}

GLuint ansimproj::Simulation::createBBoxVAO(const GLuint &vertices, const GLuint &indices) const {
  GLuint handle;
  glGenVertexArrays(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create VAO.");
  }
  glBindVertexArray(handle);

  glBindBuffer(GL_ARRAY_BUFFER, vertices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
  return handle;
}

void ansimproj::Simulation::deleteVAO(GLuint handle) {
  glDeleteVertexArrays(1, &handle);
}

void ansimproj::Simulation::resize(std::uint32_t width, std::uint32_t height) {
  glViewport(0, 0, width, height);
  height_ = height;
  width_ = width;
  deleteFBO(fbo1_);
  deleteFBO(fbo2_);
  deleteFBO(fbo3_);
  deleteTexture(texDepth_);
  deleteTexture(texColor_);
  deleteTexture(texTemp1_);
  deleteTexture(texTemp2_);
  texDepth_ = createDepthTexture(width, height);
  texColor_ = createRGB32FColorTexture(width, height);
  texTemp1_ = createR32FColorTexture(width, height);
  texTemp2_ = createR32FColorTexture(width, height);
  fbo1_ = createFullFBO(texDepth_, {texColor_});
  fbo2_ = createFlatFBO(texTemp1_);
  fbo3_ = createFlatFBO(texTemp2_);
}

ansimproj::Simulation::SimulationOptions &ansimproj::Simulation::options() {
  return options_;
}

const ansimproj::Simulation::SimulationTime &ansimproj::Simulation::time() const {
  return time_;
}
