#pragma once

#include <vector>
#include <glbinding/gl/gl.h>

namespace ansimproj {

  class Renderer {

  public:
    Renderer();

    ~Renderer();

  private:
    static void glDebugOutput(::gl::GLenum source, ::gl::GLenum type, ::gl::GLuint id,
      ::gl::GLenum severity, ::gl::GLsizei length, const ::gl::GLchar *message,
      const void *userParam);

  public:
    void render() const;

  private:
    ::gl::GLuint createShader(const std::vector<char> &vertSource, const std::vector<char> &fragSource) const;

    void deleteShader(const ::gl::GLuint &handle) const;

  private:
    ::gl::GLint versionMajor_;
    ::gl::GLint versionMinor_;
  };
}
