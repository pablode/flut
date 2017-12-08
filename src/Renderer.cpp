#include "Renderer.hpp"

#include <iostream>

using namespace gl;

ansimproj::Renderer::Renderer()
  : BaseRenderer() {

  const auto &vert = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.vert");
  const auto &frag = core::Utils::loadFileText(RESOURCES_PATH "/simplePoint.frag");
  renderProgram_ = createVertFragShader(vert, frag);
  std::cout << "Vert/Frag Shader compiled: " << renderProgram_ << std::endl;

  // clang-format off
  const std::vector<float> data = {
    //   position    |      color
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
  };
  // clang-format on
  exampleBuffer_ = createBuffer(data);
  std::cout << "Uploaded buffer: " << exampleBuffer_ << std::endl;

  const auto &comp = core::Utils::loadFileText(RESOURCES_PATH "/simpleMod.comp");
  computeProgram_ = createComputeShader(comp);
  std::cout << "Compute Shader compiled: " << computeProgram_ << std::endl;
}

ansimproj::Renderer::~Renderer() {
  deleteShader(renderProgram_);
  deleteShader(computeProgram_);
  deleteBuffer(exampleBuffer_);
}

void ansimproj::Renderer::render() const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
