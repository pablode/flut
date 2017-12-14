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
  gridPairs_ = createBuffer(gridPairsData, true);

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
  position1_ = createBuffer(posColData, true);
  position2_ = createBuffer(posColData, true);

  std::vector<float> zeroFloatData;
  zeroFloatData.resize(PARTICLE_COUNT * 3);
  velocity1_ = createBuffer(zeroFloatData, true);
  velocity2_ = createBuffer(zeroFloatData, true);
  density1_ = createBuffer(zeroFloatData, true);
  density2_ = createBuffer(zeroFloatData, true);

  std::vector<GLuint> gridIndicesData;
  gridIndicesData.resize(10 * 10 * 10 * 2);
  gridIndices_ = createBuffer(gridIndicesData, true);

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
  wallweight_ = createBuffer(wallWeightData, true);

  std::vector<float> distData;
  distData.resize(10 * 10 * 10);
  distance_ = create3DTexture(10, 10, 10, GL_R32F, GL_R, GL_FLOAT, zeroFloatData);

  // Shaders
  auto shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridInsert.comp");
  gridInsertProgram_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridSort.comp");
  gridSortProgram_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridIndexing.comp");
  gridIndexingProgram_ = createComputeShader(shaderSource);
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/positionUpdate.comp");
  positionUpdateProgram_ = createComputeShader(shaderSource);
  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.frag");
  renderProgram_ = createVertFragShader(vert, frag);

  // Other
  vao_ = createVAO(position1_);
  glPointSize(7.5f);
}

ansimproj::Simulation::~Simulation() {
  deleteShader(renderProgram_);
  deleteShader(positionUpdateProgram_);
  deleteShader(gridInsertProgram_);
  deleteShader(gridSortProgram_);
  deleteShader(gridIndexingProgram_);
  deleteBuffer(gridPairs_);
  deleteBuffer(gridIndices_);
  deleteBuffer(position1_);
  deleteBuffer(position2_);
  deleteBuffer(velocity1_);
  deleteBuffer(velocity2_);
  deleteBuffer(density1_);
  deleteBuffer(density2_);
  deleteBuffer(wallweight_);
  deleteTexture(distance_);
  deleteVAO(vao_);
}

void ansimproj::Simulation::render(const ansimproj::core::Camera &camera, float dt) {
  constexpr auto localSize = 128;
  constexpr auto numWorkGroups = PARTICLE_COUNT / localSize;

  // 1.1 Generate Particle/Voxel mappings
  glUseProgram(gridInsertProgram_);
  glProgramUniform3f(gridInsertProgram_, 0, 1.0f, 1.0f, 1.0f);
  glProgramUniform3f(gridInsertProgram_, 1, -0.5f, -0.5f, -0.5f);
  glProgramUniform3ui(gridInsertProgram_, 2, 10, 10, 10);
  glProgramUniform1ui(gridInsertProgram_, 3, PARTICLE_COUNT);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, position1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gridPairs_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // 1.2 Sort Particle/Voxel mappings
  // TODO: bitonic mergesort can only handle 2^N (power of two)!
  glUseProgram(gridSortProgram_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gridPairs_);
  const auto N = PARTICLE_COUNT;
  for (std::uint32_t size = 2; size <= N; size *= 2) {
    for (std::uint32_t stride = size / 2; stride > 0; stride /= 2) {
      glProgramUniform1ui(gridSortProgram_, 0, size);
      glProgramUniform1ui(gridSortProgram_, 1, stride);
      glDispatchCompute(numWorkGroups, 1, 1);
      glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }
  }

  // 1.3 Find Voxel indices and size
  constexpr auto searchDepth = static_cast<std::uint32_t>(std::log2(PARTICLE_COUNT)) + 1;
  glUseProgram(gridIndexingProgram_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gridPairs_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, gridIndices_);
  glProgramUniform1ui(gridIndexingProgram_, 0, searchDepth);
  glDispatchCompute(1, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);


  // TODO: Step 2 & 3


  // 4. Position Update
  glUseProgram(positionUpdateProgram_);
  glProgramUniform1f(positionUpdateProgram_, 0, dt);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, swapTextures_ ? 0 : 2, position1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocity2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, swapTextures_ ? 2 : 0, position2_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  swapTextures_ = !swapTextures_;

  // 5. Rendering
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glUseProgram(renderProgram_);
  const auto &view = camera.view();
  const auto &projection = camera.projection();
  const Eigen::Matrix4f mvp = projection * view;
  glProgramUniformMatrix4fv(renderProgram_, 0, 1, GL_FALSE, mvp.data());
  glProgramUniformMatrix4fv(renderProgram_, 1, 1, GL_FALSE, view.data());
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
