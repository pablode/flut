#pragma once

#include <glbinding/gl/gl.h>
#include <vector>

#include "core/Camera.hpp"
#include "core/Utils.hpp"

namespace ansimproj {
  namespace core {

    class BaseRenderer {

    public:
      BaseRenderer();

      virtual ~BaseRenderer();

    private:
      static void glDebugOutput(::gl::GLenum source, ::gl::GLenum type, ::gl::GLuint id,
        ::gl::GLenum severity, ::gl::GLsizei length, const ::gl::GLchar *message,
        const void *userParam);

    public:
      virtual void render(const Camera &camera, float dt) = 0;

    protected:
      ::gl::GLuint createVertFragShader(
        const std::vector<char> &vertSource, const std::vector<char> &fragSource) const;

      ::gl::GLuint createComputeShader(const std::vector<char> &shaderSource) const;

      ::gl::GLuint createBuffer(const std::vector<float> &data, bool dynamic) const;

      ::gl::GLuint createBuffer(const std::vector<::gl::GLuint> &data, bool dynamic) const;

      ::gl::GLuint create1DTexture(std::uint32_t width, ::gl::GLenum internalFormat,
        ::gl::GLenum format, ::gl::GLenum type, const std::vector<float> &data) const;

      ::gl::GLuint create3DTexture(std::uint32_t width, std::uint32_t height, std::uint32_t depth,
        ::gl::GLenum internalFormat, ::gl::GLenum format, gl::GLenum type,
        const std::vector<float> &data) const;

      void deleteShader(const ::gl::GLuint &handle) const;

      void deleteBuffer(const ::gl::GLuint &handle) const;

      void deleteTexture(const ::gl::GLuint &handle) const;

    private:
      ::gl::GLint versionMajor_;
      ::gl::GLint versionMinor_;
    };
  }
}
