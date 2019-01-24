#pragma once

#include <cstdint>
#include <glbinding/gl/gl.h>
#include <vector>

#include "core/Camera.hpp"
#include "core/Utils.hpp"

namespace ansimproj {
  namespace core {

    class SimulationBase {

    public:
      SimulationBase();

      virtual ~SimulationBase();

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

      ::gl::GLuint createFullFBO(
        const ::gl::GLuint &depthTexture, std::vector<::gl::GLuint> colorTextures) const;

      ::gl::GLuint createFlatFBO(const ::gl::GLuint &colorTexture) const;

      ::gl::GLuint createRGB32FColorTexture(
        const std::uint32_t &width, const std::uint32_t &height) const;

      ::gl::GLuint createR32FColorTexture(
        const std::uint32_t &width, const std::uint32_t &height) const;

      ::gl::GLuint createDepthTexture(
        const std::uint32_t &width, const std::uint32_t &height) const;

      void deleteShader(const ::gl::GLuint &handle) const;

      void deleteBuffer(const ::gl::GLuint &handle) const;

      void deleteFBO(const ::gl::GLuint &handle) const;

      void deleteTexture(const ::gl::GLuint &handle) const;

    private:
      ::gl::GLuint createColorTexture(::gl::GLenum internalFormat, const std::uint32_t &width, const std::uint32_t &height) const;

    private:
      ::gl::GLint versionMajor_;
      ::gl::GLint versionMinor_;
    };
  }
}
