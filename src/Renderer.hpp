#pragma once

#include "core/BaseRenderer.hpp"

namespace ansimproj {

  class Renderer : public core::BaseRenderer {

  public:
    Renderer();

    ~Renderer();

    void render(const core::Camera &camera) const override;

    void resize(std::uint32_t width, std::uint32_t height);

  private:
    ::gl::GLuint createVAO(const ::gl::GLuint &vbo) const;

    void deleteVAO(::gl::GLuint handle);

  private:
    ::gl::GLuint renderProgram_;
    ::gl::GLuint computeProgram_;
    ::gl::GLuint testBuffer_;
    ::gl::GLuint vao_;
  };
}
