#include "Simulation.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>
#include <limits>

using namespace gl;

ansimproj::Simulation::Simulation()
  : BaseRenderer()
  , swapTextures_{false} {

  // Buffers
  std::vector<GLuint> gridPairsData;
  gridPairsData.resize(PARTICLE_COUNT * 2);
  bufGridPairs_ = createBuffer(gridPairsData, true);

  std::vector<float> posColData;
  posColData.reserve(PARTICLE_COUNT * 6);
  for (std::uint32_t i = 0; i < PARTICLE_COUNT; ++i) {
    const float x = (std::rand() % 10000) / 10000.0f;
    const float y = (std::rand() % 10000) / 10000.0f;
    const float z = (std::rand() % 10000) / 10000.0f;
    posColData.push_back(-0.5f + x);
    posColData.push_back(-0.5f + y);
    posColData.push_back(-0.5f + z);
    posColData.push_back(x);
    posColData.push_back(y);
    posColData.push_back(z);
  }
  bufPosition1_ = createBuffer(posColData, true);
  bufPosition2_ = createBuffer(posColData, true);

  std::vector<float> zeroFloatData;
  zeroFloatData.resize(PARTICLE_COUNT * 3);
  bufVelocity1_ = createBuffer(zeroFloatData, true);
  bufVelocity2_ = createBuffer(zeroFloatData, true);
  bufDensity1_ = createBuffer(zeroFloatData, true);
  bufDensity2_ = createBuffer(zeroFloatData, true);

  std::vector<GLuint> gridIndicesData;
  gridIndicesData.resize(10 * 10 * 10 * 2);
  bufGridIndices_ = createBuffer(gridIndicesData, true);

  std::vector<float> wallWeightData;
  wallWeightData.reserve(1024);
  wallWeightData.push_back(std::numeric_limits<float>::lowest());
  const float re = 0.05;            // effective range
  for (auto i = 1; i < 1024; ++i) { // 1024 samples
    const float r = 1.0f / i;       // range [0, 1]
    const double weight =
      (315 * std::pow(std::pow(re, 2) - std::pow(r, 2), 3)) / (64 * M_PI * std::pow(re, 9));
    wallWeightData.push_back(static_cast<float>(weight));
  }
  bufWallweight_ = createBuffer(wallWeightData, true);

  std::vector<float> distData;
  distData.reserve(10 * 10 * 10);
  for (auto x = 0; x < 10; ++x) {
    for (auto y = 0; y < 10; ++y) {
      for (auto z = 0; z < 10; ++z) {
        const auto i = std::min(x, std::min(y, z));
        const float d = std::abs(std::abs(5 - i) - 5) / 5.0f;
        distData.push_back(d);
      }
    }
  }
  texDistance_ = create3DTexture(10, 10, 10, GL_R32F, GL_R, GL_FLOAT, distData);

  // Shaders
  auto shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridInsert.comp");
  programGridInsert_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridSort.comp");
  programGridSort_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridIndexing.comp");
  programGridIndexing_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/densityComputation.comp");
  programDensityComputation_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/velocityUpdate.comp");
  programVelocityUpdate_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/positionUpdate.comp");
  programPositionUpdate_ = createComputeShader(shaderSource);
  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simpleColor.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simpleColor.frag");
  programRender_ = createVertFragShader(vert, frag);

  // Other
  vao_ = createVAO(bufPosition1_);
  glPointSize(7.5f);
}

ansimproj::Simulation::~Simulation() {
  deleteShader(programGridInsert_);
  deleteShader(programGridSort_);
  deleteShader(programGridIndexing_);
  deleteShader(programDensityComputation_);
  deleteShader(programVelocityUpdate_);
  deleteShader(programPositionUpdate_);
  deleteShader(programRender_);
  deleteBuffer(bufGridPairs_);
  deleteBuffer(bufGridIndices_);
  deleteBuffer(bufPosition1_);
  deleteBuffer(bufPosition2_);
  deleteBuffer(bufVelocity1_);
  deleteBuffer(bufVelocity2_);
  deleteBuffer(bufDensity1_);
  deleteBuffer(bufDensity2_);
  deleteBuffer(bufWallweight_);
  deleteTexture(texDistance_);
  deleteVAO(vao_);
}

void ansimproj::Simulation::render(const ansimproj::core::Camera &camera, float dt) {
  constexpr auto localSize = 128;
  constexpr auto numWorkGroups = PARTICLE_COUNT / localSize;
  swapTextures_ = !swapTextures_;

  // 1.1 Generate Particle/Voxel mappings
  glUseProgram(programGridInsert_);
  glProgramUniform3f(programGridInsert_, 0, 1.0f, 1.0f, 1.0f);
  glProgramUniform3f(programGridInsert_, 1, -0.5f, -0.5f, -0.5f);
  glProgramUniform3ui(programGridInsert_, 2, 10, 10, 10);
  glProgramUniform1ui(programGridInsert_, 3, PARTICLE_COUNT);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufPosition1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridPairs_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // 1.2 Sort Particle/Voxel mappings
  // TODO: bitonic mergesort can only handle 2^N!
  glUseProgram(programGridSort_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufGridPairs_);
  const auto N = PARTICLE_COUNT;
  for (std::uint32_t size = 2; size <= N; size *= 2) {
    for (std::uint32_t stride = size / 2; stride > 0; stride /= 2) {
      glProgramUniform1ui(programGridSort_, 0, size);
      glProgramUniform1ui(programGridSort_, 1, stride);
      glDispatchCompute(numWorkGroups, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
  }

  // 1.3 Find Voxel indices and size
  const auto searchDepth = static_cast<std::uint32_t>(std::log2(PARTICLE_COUNT)) + 1;
  glUseProgram(programGridIndexing_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, bufGridPairs_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridIndices_);
  glProgramUniform1ui(programGridIndexing_, 0, searchDepth);
  glDispatchCompute(1, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // 2. Density Computation
  // TODO: implement shader
  glUseProgram(programDensityComputation_);
  glProgramUniform3f(programDensityComputation_, 0, 1.0f, 1.0f, 1.0f);
  glProgramUniform3f(programDensityComputation_, 1, -0.5f, -0.5f, -0.5f);
  glProgramUniform3ui(programDensityComputation_, 2, 10, 10, 10);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapTextures_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridPairs_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufGridIndices_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, swapTextures_ ? bufDensity1_ : bufDensity2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bufWallweight_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, swapTextures_ ? bufDensity2_ : bufDensity1_);
  glBindImageTexture(0, texDistance_, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // 3. Velocity Update
  // TODO: implement shader
  glUseProgram(programVelocityUpdate_);
  glProgramUniform1f(programVelocityUpdate_, 0, dt);
  glProgramUniform3f(programVelocityUpdate_, 1, 1.0f, 1.0f, 1.0f);
  glProgramUniform3f(programVelocityUpdate_, 2, -0.5f, -0.5f, -0.5f);
  glProgramUniform3ui(programVelocityUpdate_, 3, 10, 10, 10);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapTextures_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, bufGridPairs_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, bufGridIndices_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, swapTextures_ ? bufDensity2_ : bufDensity1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, swapTextures_ ? bufVelocity1_ : bufVelocity2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, swapTextures_ ? bufVelocity2_ : bufVelocity1_);
  glBindImageTexture(0, texDistance_, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // 4. Position Update
  glUseProgram(programPositionUpdate_);
  glProgramUniform1f(programPositionUpdate_, 0, dt);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, swapTextures_ ? bufPosition1_ : bufPosition2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, swapTextures_ ? bufVelocity2_ : bufVelocity1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, swapTextures_ ? bufPosition2_ : bufPosition1_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // 5. Rendering
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(programRender_);
  const auto &view = camera.view();
  const auto &projection = camera.projection();
  const Eigen::Matrix4f mvp = projection * view;
  glProgramUniformMatrix4fv(programRender_, 0, 1, GL_FALSE, mvp.data());
  glProgramUniformMatrix4fv(programRender_, 1, 1, GL_FALSE, view.data());
  glBindVertexArray(vao_);
  glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
}

GLuint ansimproj::Simulation::createVAO(const GLuint &vbo) const {
  GLuint handle;
  glCreateVertexArrays(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create VAO.");
  }
  constexpr GLuint binding = 0;
  glVertexArrayVertexBuffer(handle, binding, vbo, 0, 6 * sizeof(float));

  constexpr GLuint posAttrIndex = 0;
  constexpr GLuint posAttrOffet = 0;
  glEnableVertexArrayAttrib(handle, posAttrIndex);
  glVertexArrayAttribFormat(handle, posAttrIndex, 3, GL_FLOAT, GL_FALSE, posAttrOffet);
  glVertexArrayAttribBinding(handle, posAttrIndex, binding);
  constexpr GLuint colAttrIndex = 1;
  constexpr GLuint colAttrOffet = 3 * sizeof(float);
  glEnableVertexArrayAttrib(handle, colAttrIndex);
  glVertexArrayAttribFormat(handle, colAttrIndex, 3, GL_FLOAT, GL_FALSE, colAttrOffet);
  glVertexArrayAttribBinding(handle, colAttrIndex, binding);
  return handle;
}

void ansimproj::Simulation::deleteVAO(GLuint handle) {
  glDeleteVertexArrays(1, &handle);
}

void ansimproj::Simulation::resize(std::uint32_t width, std::uint32_t height) {
  glViewport(0, 0, width, height);
}
