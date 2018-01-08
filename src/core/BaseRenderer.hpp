#pragma once

#include <GL/glew.h>
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
      static void glDebugOutput(GLenum source, GLenum type, GLuint id,
        GLenum severity, GLsizei length, const GLchar *message,
        const void *userParam);

    public:
      virtual void render(const Camera &camera, float dt) = 0;

    protected:
      GLuint createVertFragShader(
        const std::vector<char> &vertSource, const std::vector<char> &fragSource) const;

      GLuint createComputeShader(const std::vector<char> &shaderSource) const;

      GLuint createBuffer(const std::vector<float> &data, bool dynamic) const;

      GLuint createBuffer(const std::vector<GLuint> &data, bool dynamic) const;

      GLuint create1DTexture(std::uint32_t width, GLenum internalFormat,
        GLenum format, GLenum type, const std::vector<float> &data) const;

      GLuint create3DTexture(std::uint32_t width, std::uint32_t height, std::uint32_t depth,
        GLenum internalFormat, GLenum format, GLenum type,
        const std::vector<float> &data) const;

      void deleteShader(const GLuint &handle) const;

      void deleteBuffer(const GLuint &handle) const;

      void deleteTexture(const GLuint &handle) const;

    private:
      GLint versionMajor_;
      GLint versionMinor_;
    };
  }
}
