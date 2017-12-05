#include "Renderer.hpp"

#include <glbinding/Binding.h>
#include <glbinding/ContextInfo.h>
#include <glbinding/gl/extension.h>
#include <iostream>

using namespace gl;

ansimproj::Renderer::Renderer() {
  glbinding::Binding::initialize();
  glGetIntegerv(GL_MAJOR_VERSION, &versionMajor_);
  glGetIntegerv(GL_MINOR_VERSION, &versionMinor_);
  auto extensions = glbinding::ContextInfo::extensions();

#ifndef NDEBUG
  if ((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 3)) ||
    extensions.count(GLextension::GL_ARB_debug_output)) {
    ContextFlagMask flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, (GLint *)&flags);

    if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != GL_NONE_BIT) {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(&glDebugOutput, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
      std::cout << "Debug output enabled." << std::endl;
    } else {
      std::cout << "Debug output not available (context flag not set)." << std::endl;
    }
  } else {
    std::cout << "Debug output not available (OpenGL version < 4.3 and ARB_debug_output extension "
                 "not given)."
              << std::endl;
  }
#endif
  if ((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 5)) ||
    extensions.count(GLextension::GL_ARB_clip_control)) {
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
  } else {
    throw std::runtime_error("OpenGL version 4.5 or ARB_clip_control extension required.");
  }
  if (!((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 5)) ||
        extensions.count(GLextension::GL_ARB_direct_state_access))) {
    throw std::runtime_error("OpenGL version 4.5 or ARB_direct_state_access extension required.");
  }
  if (!((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 2)) ||
        extensions.count(GLextension::GL_ARB_texture_storage))) {
    throw std::runtime_error("OpenGL version 4.5 or ARB_texture_storage extension required.");
  }

  glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
}

ansimproj::Renderer::~Renderer() {}

void ansimproj::Renderer::glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity,
  GLsizei length, const GLchar *message, const void *userParam) {
  // clang-format off
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
  std::cout << "---------------" << std::endl;
  std::cout << "Debug message (" << id << "): " << message << std::endl;
  switch (source) {
  case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: GlfwWindow System"; break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
  case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
  case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
  default: break;
  } std::cout << std::endl;

  switch (type) {
  case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
  case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
  case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
  case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
  case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
  case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
  case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
  default: break;
  } std::cout << std::endl;

  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
  case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
  case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
  case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
  default: break;
  } std::cout << std::endl;
  std::cout << std::endl;
  // clang-format on
}

void ansimproj::Renderer::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
