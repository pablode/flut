#include "Simulation.hpp"

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <iostream>

using namespace gl;

ansimproj::Simulation::Simulation()
  : BaseRenderer() {

  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.frag");
  renderProgram_ = createVertFragShader(vert, frag);
  std::cout << "Vert/Frag Shader compiled: " << renderProgram_ << std::endl;

  std::vector<float> data;
  const std::uint64_t dataSize = PARTICLE_COUNT * 6;
  data.reserve(dataSize);
  const std::uint64_t AXIS_COUNT = static_cast<std::uint64_t>(std::cbrt(PARTICLE_COUNT));
  assert((std::cbrt(PARTICLE_COUNT) - AXIS_COUNT) < 0.00001);
  for (std::uint64_t x = 0; x < AXIS_COUNT; ++x) {
    for (std::uint64_t y = 0; y < AXIS_COUNT; ++y) {
      for (std::uint64_t z = 0; z < AXIS_COUNT; ++z) {
        const float valX = -0.5f + (static_cast<float>(x) / AXIS_COUNT);
        const float valY = -0.5f + (static_cast<float>(y) / AXIS_COUNT);
        const float valZ = -0.5f + (static_cast<float>(z) / AXIS_COUNT);
        const float colX = static_cast<float>(x) / AXIS_COUNT;
        const float colY = static_cast<float>(y) / AXIS_COUNT;
        const float colZ = static_cast<float>(z) / AXIS_COUNT;
        data.push_back(valX);
        data.push_back(valY);
        data.push_back(valZ);
        data.push_back(colX);
        data.push_back(colY);
        data.push_back(colZ);
      }
    }
  }

  testBuffer_ = createBuffer(data, true);
  std::cout << "Uploaded buffer: " << testBuffer_ << std::endl;

  const auto &comp = core::Utils::loadFileText(RESOURCES_PATH "/positionUpdate.comp");
  computeProgram_ = createComputeShader(comp);
  std::cout << "Compute Shader compiled: " << computeProgram_ << std::endl;

  vao_ = createVAO(testBuffer_);
  std::cout << "VAO created: " << vao_ << std::endl;

  std::vector<float> data2;
  data2.resize(10 * 10 * 10 * 4);
  test3dTex_ = create3DTexture(10, 10, 10, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, data2);

  glPointSize(5.0f);
}

ansimproj::Simulation::~Simulation() {
  deleteShader(renderProgram_);
  deleteShader(computeProgram_);
  deleteBuffer(testBuffer_);
  deleteVAO(vao_);
  deleteTexture(test3dTex_);
}

void ansimproj::Simulation::render(const ansimproj::core::Camera &camera) const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindVertexArray(vao_);

  // Use compute shader to modify test data
  constexpr bool useCompute = false;
  if (useCompute) {
    const Eigen::Vector3f color{1.0f, 0.0f, 0.0};
    constexpr auto workGroupSize = 10;
    glUseProgram(computeProgram_);
    glProgramUniform3f(computeProgram_, 0, color.x(), color.y(), color.z());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, testBuffer_);
    glDispatchCompute(PARTICLE_COUNT / workGroupSize, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // Simple vert/frag rendering program
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
