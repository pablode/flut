#pragma once

#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace flut
{
  class GlHelper
  {
  public:
    struct ShaderDefine
    {
      ShaderDefine(std::string_view name, uint32_t value)
        : name(name)
      {
        valueStr = std::to_string(value);
      }
      ShaderDefine(std::string_view name, float value)
        : name(name)
      {
        valueStr = std::to_string(value);
      }
      ShaderDefine(std::string_view name, glm::ivec3 value)
        : name(name)
      {
        std::stringstream ss;
        ss << "ivec3(" << value.x << ", " << value.y << ", " << value.z << ")";
        valueStr = ss.str();
      }
      ShaderDefine(std::string_view name, glm::vec3 value)
        : name(name)
      {
        std::stringstream ss;
        ss << "vec3(" << value.x << ", " << value.y << ", " << value.z << ")";
        valueStr = ss.str();
      }
      std::string_view name;
      std::string valueStr;
    };

  public:
    static void enableDebugHooks();

    static GLuint createVertFragShader(const char* vertPath, const char* fragPath, std::vector<ShaderDefine> defines = {});

    static GLuint createComputeShader(const char* path, std::vector<ShaderDefine> defines = {});

  private:
    static std::string loadFileText(const std::string& filePath);

    static std::string preprocessShaderSource(const std::string& text, std::vector<ShaderDefine> defines);

    static void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
      const GLchar* message, const void* userParam);
  };
}
