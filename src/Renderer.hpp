#pragma once

#include <glbinding/gl/gl.h>

namespace ansimproj {

  class Renderer {

  public:
    Renderer();

    ~Renderer();

  public:
    void render();

  private:
    static void glDebugOutput(::gl::GLenum source, ::gl::GLenum type, ::gl::GLuint id,
      ::gl::GLenum severity, ::gl::GLsizei length, const ::gl::GLchar *message,
      const void *userParam);

  private:
    ::gl::GLint versionMajor_;
    ::gl::GLint versionMinor_;
  };
}
