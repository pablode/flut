#pragma once

#include <cstdint>
#include <glad/glad.h>
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
      static void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
        const GLchar* message, const void* userParam);

    public:
      virtual void render(const Camera& camera, float dt) = 0;

    protected:
      GLuint createVertFragShader(const std::vector<char>& vertSource, const std::vector<char>& fragSource) const;

      GLuint createComputeShader(const std::vector<char>& shaderSource) const;

      GLuint createBuffer(const std::vector<float>& data, bool dynamic) const;

      GLuint createBuffer(const std::vector<GLuint>& data, bool dynamic) const;

      GLuint createFullFBO(GLuint depthTexture, std::vector<GLuint> colorTextures) const;

      GLuint createFlatFBO(GLuint colorTexture) const;

      GLuint createRGB32FColorTexture(std::uint32_t width, std::uint32_t height) const;

      GLuint createR32FColorTexture(std::uint32_t width, std::uint32_t height) const;

      GLuint createDepthTexture(std::uint32_t width, std::uint32_t height) const;

      GLuint64 makeImageResident(GLuint handle) const;

      GLuint64 makeTextureResident(GLuint handle) const;

      void makeImageNonResident(GLuint64 bindlessHandle) const;

      void makeTextureNonResident(GLuint64 bindlessHandle) const;

      void deleteShader(GLuint handle) const;

      void deleteBuffer(GLuint handle) const;

      void deleteFBO(GLuint handle) const;

      void deleteTexture(GLuint handle) const;

    private:
      GLuint createColorTexture(GLenum internalFormat, std::uint32_t width, std::uint32_t height) const;

    private:
      GLint versionMajor_;
      GLint versionMinor_;
    };
  }
}
