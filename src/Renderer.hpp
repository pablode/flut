#pragma once

#include <glbinding/gl/gl.h>
#include <vector>

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
    ::gl::GLuint createVertFragShader(
      const std::vector<char> &vertSource, const std::vector<char> &fragSource) const;

    ::gl::GLuint createComputeShader(const std::vector<char> &shaderSource) const;

    ::gl::GLuint createBuffer(const std::vector<float> data) const;

    void deleteShader(const ::gl::GLuint &handle) const;

    void deleteBuffer(const ::gl::GLuint &handle) const;

  private:
    ::gl::GLint versionMajor_;
    ::gl::GLint versionMinor_;
  };
}
