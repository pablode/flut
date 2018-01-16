#include "Simulation.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <limits>

ansimproj::Simulation::Simulation()
  : BaseRenderer()
  , swapFrame_{false}
  , frame_{0}
  , bufColor_{0}
  , bufGridPairs_{0}
  , bufGridIndices_{0}
  , bufPosition1_{0}
  , bufVelocity1_{0}
  , bufPosition2_{0}
  , bufVelocity2_{0}
  , bufWallweight_{0}
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
  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simpleColor.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simpleColor.frag");
  programRender_ = createVertFragShader(vert, frag);

  // Buffers
  preset1();

  // Other
  glGenQueries(6, &timerQueries_[0][0]);
  glGenQueries(6, &timerQueries_[1][0]);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
}

ansimproj::Simulation::~Simulation() {
  glDeleteQueries(6, &timerQueries_[0][0]);
  glDeleteQueries(6, &timerQueries_[1][0]);
  deleteShader(programGridInsert_);
  deleteShader(programGridSort_);
  deleteShader(programGridIndexing_);
  deleteShader(programDensityComputation_);
  deleteShader(programForceUpdate_);
  deleteShader(programRender_);
  deleteBuffer(bufColor_);
  deleteBuffer(bufGridPairs_);
  deleteBuffer(bufGridIndices_);
  deleteBuffer(bufPosition1_);
  deleteBuffer(bufPosition2_);
  deleteBuffer(bufVelocity1_);
  deleteBuffer(bufVelocity2_);
  deleteBuffer(bufWallweight_);
  deleteBuffer(bufDensity_);
  deleteVAO(vao1_);
  deleteVAO(vao2_);
}

void ansimproj::Simulation::preset1() {
  if (bufGridPairs_)
    deleteBuffer(bufGridPairs_);
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
  if (bufWallweight_)
    deleteBuffer(bufWallweight_);
  if (bufDensity_)
    deleteBuffer(bufDensity_);
  if (vao1_)
    deleteVAO(vao1_);
  if (vao2_)
    deleteVAO(vao2_);

  std::vector<GLuint> gridPairsData;
  gridPairsData.resize(PARTICLE_COUNT * 2);
  bufGridPairs_ = createBuffer(gridPairsData, true);

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
    colData.push_back(1.0f - yi * 0.5f);
    colData.push_back(1.0f - yi * 0.5f);
    colData.push_back(1.0f);
  }
  bufPosition1_ = createBuffer(posData, true);
  bufPosition2_ = createBuffer(posData, true);
  bufColor_ = createBuffer(colData, false);

  std::vector<float> zeroFloatData;
  zeroFloatData.resize(PARTICLE_COUNT * 3);
  bufVelocity1_ = createBuffer(zeroFloatData, true);
  bufVelocity2_ = createBuffer(zeroFloatData, true);
  bufDensity_ = createBuffer(zeroFloatData, true);

  // TODO: using a/this weight function is not correct
  const std::uint32_t numSamples = 1024;
  std::vector<float> wallWeightData;
  wallWeightData.reserve(numSamples);
  wallWeightData.push_back(std::numeric_limits<float>::lowest());
  const float D = 1;
  const float D_r = std::sqrt(D);
  for (std::uint32_t i = 1; i <= numSamples; ++i) {
    const float r = 1.0f / i;
    const auto weight = std::pow(D_r, r * 2);
    wallWeightData.push_back(weight);
  }
  bufWallweight_ = createBuffer(wallWeightData, true);

  vao1_ = createVAO(bufPosition1_, bufColor_);
  vao2_ = createVAO(bufPosition2_, bufColor_);
}

void ansimproj::Simulation::render(const ansimproj::core::Camera &camera, float dt) {
  constexpr auto localSize = 128;
  const auto numWorkGroups = static_cast<std::uint32_t>(std::ceil(PARTICLE_COUNT / localSize));
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
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridPairs_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glEndQuery(GL_TIME_ELAPSED);

  // 1.2 Sort Particle/Voxel mappings
  // TODO: replace bitonic mergesort with counting sort
  glBeginQuery(GL_TIME_ELAPSED, query[1]);
  glUseProgram(programGridSort_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufGridPairs_);
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
  const auto searchDepth = static_cast<std::uint32_t>(std::log2(PARTICLE_COUNT)) + 1;
  const auto indexingWorkGroups =
    static_cast<std::uint32_t>(std::ceil(GRID_VOXEL_COUNT / static_cast<float>(localSize)));
  glBeginQuery(GL_TIME_ELAPSED, query[2]);
  glUseProgram(programGridIndexing_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufGridPairs_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridIndices_);
  glProgramUniform1ui(programGridIndexing_, 0, searchDepth);
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
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridPairs_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufGridIndices_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufWallweight_);
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
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridPairs_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufGridIndices_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bufDensity_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, swapFrame_ ? bufVelocity1_ : bufVelocity2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, swapFrame_ ? bufVelocity2_ : bufVelocity1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, swapFrame_ ? bufPosition2_ : bufPosition1_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  glEndQuery(GL_TIME_ELAPSED);

  // 4. Rendering
  glBeginQuery(GL_TIME_ELAPSED, query[5]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  const float pointRadius = options_.shadingMode == 0 ? 25.0f : 50.0f;
  glUseProgram(programRender_);
  const auto &view = camera.view();
  const auto &projection = camera.projection();
  const Eigen::Matrix4f mvp = projection * view;
  glProgramUniformMatrix4fv(programRender_, 0, 1, GL_FALSE, mvp.data());
  glProgramUniformMatrix4fv(programRender_, 1, 1, GL_FALSE, view.data());
  glProgramUniformMatrix4fv(programRender_, 2, 1, GL_FALSE, projection.data());
  glProgramUniform3fv(programRender_, 3, 1, GRID_LEN.data());
  glProgramUniform3fv(programRender_, 4, 1, GRID_ORIGIN.data());
  glProgramUniform3uiv(programRender_, 5, 1, GRID_RES.data());
  glProgramUniform1ui(programRender_, 6, PARTICLE_COUNT);
  glProgramUniform1f(programRender_, 7, pointRadius);
  glProgramUniform1i(programRender_, 8, options_.colorMode);
  glProgramUniform1i(programRender_, 9, options_.shadingMode);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapFrame_ ? bufPosition2_ : bufPosition1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufDensity_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, swapFrame_ ? bufVelocity2_ : bufVelocity1_);
  glBindVertexArray(swapFrame_ ? vao2_ : vao1_);
  glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
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

GLuint ansimproj::Simulation::createVAO(const GLuint &vboPos, const GLuint& vboCol) const {
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

void ansimproj::Simulation::deleteVAO(GLuint handle) {
  glDeleteVertexArrays(1, &handle);
}

void ansimproj::Simulation::resize(std::uint32_t width, std::uint32_t height) {
  glViewport(0, 0, width, height);
}

ansimproj::Simulation::SimulationOptions &ansimproj::Simulation::options() {
  return options_;
}

const ansimproj::Simulation::SimulationTime &ansimproj::Simulation::time() const {
  return time_;
}
