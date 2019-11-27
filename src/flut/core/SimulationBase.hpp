#pragma once

#include <cstdint>
#include <glbinding/gl/gl.h>
#include <vector>

#include "Camera.hpp"

namespace flut
{
  namespace core
  {
    class SimulationBase
    {
    public:
      SimulationBase();

      virtual ~SimulationBase();

    private:
      static void glDebugOutput(::gl::GLenum source, ::gl::GLenum type, ::gl::GLuint id, ::gl::GLenum severity,
        ::gl::GLsizei length, const ::gl::GLchar* message, const void* userParam);

    public:
      virtual void render(const Camera& camera, float dt) = 0;

    protected:
      ::gl::GLuint createVertFragShader(const std::vector<char>& vertSource, const std::vector<char>& fragSource) const;

      ::gl::GLuint createComputeShader(const std::vector<char>& shaderSource) const;

      ::gl::GLuint createBuffer(const std::vector<float>& data, bool dynamic) const;

      ::gl::GLuint createBuffer(const std::vector<::gl::GLuint>& data, bool dynamic) const;

      ::gl::GLuint createFullFBO(::gl::GLuint depthTexture, std::vector<::gl::GLuint> colorTextures) const;

      ::gl::GLuint createFlatFBO(::gl::GLuint colorTexture) const;

      ::gl::GLuint createRGB32FColorTexture(std::uint32_t width, std::uint32_t height) const;

      ::gl::GLuint createR32FColorTexture(std::uint32_t width, std::uint32_t height) const;

      ::gl::GLuint createDepthTexture(std::uint32_t width, std::uint32_t height) const;

      ::gl::GLuint64 makeTextureResident(::gl::GLuint handle) const;

      void makeTextureNonResident(::gl::GLuint64 bindlessHandle) const;

      void deleteShader(::gl::GLuint handle) const;

      void deleteBuffer(::gl::GLuint handle) const;

      void deleteFBO(::gl::GLuint handle) const;

      void deleteTexture(::gl::GLuint handle) const;

    private:
      ::gl::GLuint createColorTexture(::gl::GLenum internalFormat, std::uint32_t width, std::uint32_t height) const;

    private:
      ::gl::GLint versionMajor_;
      ::gl::GLint versionMinor_;
    };
  }
}
