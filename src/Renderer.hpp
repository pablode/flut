#pragma once

#include "BaseRenderer.hpp"

namespace ansimproj {
  class Renderer : public BaseRenderer {

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
