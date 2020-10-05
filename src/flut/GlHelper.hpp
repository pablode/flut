#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>

class GlHelper
{
public:
  static void enableDebugHooks();

  static GLuint createVertFragShader(const char* vertPath, const char* fragPath);

  static GLuint createComputeShader(const char* path);

private:
  static void loadFileText(const std::string& filePath, std::vector<char>& text);

  static void glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);
};
