#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Renderer : public core::BaseRenderer {

  public:
    Renderer();

    ~Renderer();

    void render() const override;

  private:
    ::gl::GLuint renderProgram_;
    ::gl::GLuint computeProgram_;
    ::gl::GLuint exampleBuffer_;
  };
}
