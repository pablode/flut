#include "Simulation.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>

using namespace gl;

ansimproj::Simulation::Simulation()
  : BaseRenderer() {

  // Buffer with <uint particleId, uint voxelId>
  // representing the Uniform Grid mappings.
  std::vector<GLuint> gridPairsData;
  gridPairsData.resize(PARTICLE_COUNT * 2);
  gridPairs_ = createBuffer(gridPairsData, true);

  // Position 1 & 2 Buffers (pos+col)
  std::vector<float> positionData;
  positionData.reserve(PARTICLE_COUNT * 6);
  for (std::uint32_t i = 0; i < PARTICLE_COUNT; ++i) {
    const float x = (rand() % 10000) / 10000.0f;
    const float y = (rand() % 10000) / 10000.0f;
    const float z = (rand() % 10000) / 10000.0f;
    positionData.push_back(-0.5f + x);
    positionData.push_back(-0.5f + y);
    positionData.push_back(-0.5f + z);
    positionData.push_back(x);
    positionData.push_back(y);
    positionData.push_back(z);
  }
  position1_ = createBuffer(positionData, true);
  position2_ = createBuffer(positionData, true);

  // Velocity Buffers
  std::vector<float> velocityData;
  velocityData.reserve(PARTICLE_COUNT * 3);
  for (std::uint64_t x = 0; x < PARTICLE_COUNT * 3; ++x) {
    velocityData.push_back(0.0f);
  }
  velocity2_ = createBuffer(velocityData, true);

  // Grid Lookup Buffer
  std::vector<GLuint> temp;
  temp.resize(10 * 10 * 10 * 2);
  gridIndices_ = createBuffer(temp, true);

  std::cout << "Uploaded buffers: " << position1_ << ", " << position2_ << ", " << velocity2_
            << ", " << gridIndices_ << std::endl;

  // Uniform Grid Insert Shader
  auto shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridInsert.comp");
  gridInsertProgram_ = createComputeShader(shaderSource);
  std::cout << "Grid Insert Shader compiled: " << gridInsertProgram_ << std::endl;

  // Uniform Grid Sort Shader
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridSort.comp");
  gridSortProgram_ = createComputeShader(shaderSource);
  std::cout << "Grid Sort Shader compiled: " << gridSortProgram_ << std::endl;

  // Uniform Grid Indexing Shader
  shaderSource = core::Utils::loadFileText(RESOURCES_PATH "/gridIndexing.comp");
  gridIndexingProgram_ = createComputeShader(shaderSource);
  std::cout << "Grid Indexing Shader compiled: " << gridSortProgram_ << std::endl;

  // Position Update Compute Shader
  const auto &comp3 = core::Utils::loadFileText(RESOURCES_PATH "/positionUpdate.comp");
  positionUpdateProgram_ = createComputeShader(comp3);
  std::cout << "Position Update Shader compiled: " << positionUpdateProgram_ << std::endl;

  // Render shader & VAO
  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.frag");
  renderProgram_ = createVertFragShader(vert, frag);
  std::cout << "Render Shader compiled: " << renderProgram_ << std::endl;
  vao_ = createVAO(position1_);
  std::cout << "VAO created: " << vao_ << std::endl;

  swapTextures_ = false;
  glPointSize(7.5f);
}

ansimproj::Simulation::~Simulation() {
  deleteShader(renderProgram_);
  deleteShader(positionUpdateProgram_);
  deleteShader(gridInsertProgram_);
  deleteShader(gridSortProgram_);
  deleteShader(gridIndexingProgram_);
  deleteBuffer(position1_);
  deleteBuffer(position2_);
  deleteBuffer(velocity2_);
  deleteBuffer(gridPairs_);
  deleteBuffer(gridIndices_);
  deleteVAO(vao_);
}

void ansimproj::Simulation::render(const ansimproj::core::Camera &camera, float dt) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vao_);

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


  // 4. Position Update Shader test
  glUseProgram(positionUpdateProgram_);
  glProgramUniform1f(positionUpdateProgram_, 0, dt);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, swapTextures_ ? 0 : 2, position1_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, velocity2_);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, swapTextures_ ? 2 : 0, position2_);
  glDispatchCompute(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  swapTextures_ = !swapTextures_;

  // 5. Simple vert/frag rendering program
  glUseProgram(renderProgram_);
  const auto &view = camera.view();
  const auto &projection = camera.projection();
  constexpr GLuint mvpLoc = 0;
  constexpr GLuint mvLoc = 1;
  const Eigen::Matrix4f mvp = projection * view;
  glProgramUniformMatrix4fv(renderProgram_, mvpLoc, 1, GL_FALSE, mvp.data());
  glProgramUniformMatrix4fv(renderProgram_, mvLoc, 1, GL_FALSE, view.data());
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
