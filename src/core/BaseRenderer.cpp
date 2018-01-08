#include "BaseRenderer.hpp"

#include <iostream>

ansimproj::core::BaseRenderer::BaseRenderer() {
  glGetIntegerv(GL_MAJOR_VERSION, &versionMajor_);
  glGetIntegerv(GL_MINOR_VERSION, &versionMinor_);

#ifndef NDEBUG
  if ((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 3)) || GLEW_ARB_debug_output) {
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
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
  if ((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 5)) || GLEW_ARB_clip_control) {
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
  } else {
    throw std::runtime_error("OpenGL version 4.5 or ARB_clip_control extension required.");
  }
  if (!((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 5)) ||
        GLEW_ARB_direct_state_access)) {
    throw std::runtime_error("OpenGL version 4.5 or ARB_direct_state_access extension required.");
  }
  if (!((versionMajor_ > 4 || (versionMajor_ == 4 && versionMinor_ >= 2)) ||
        GLEW_ARB_texture_storage)) {
    throw std::runtime_error("OpenGL version 4.2 or ARB_texture_storage extension required.");
  }
  if (!((versionMajor_ > 4 || (versionMajor_ == 4 && versionMajor_ >= 3)) ||
        GLEW_ARB_compute_shader)) {
    throw std::runtime_error("OpenGL version 4.3 or ARB_compute_shader extension required.");
  }
  if (!((versionMajor_ > 4 || (versionMajor_ == 4 && versionMajor_ >= 3)) ||
        GLEW_ARB_explicit_uniform_location)) {
    throw std::runtime_error(
      "OpenGL version 4.3 or ARB_explicit_uniform_location extension required.");
  }

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
}

ansimproj::core::BaseRenderer::~BaseRenderer() {}

void ansimproj::core::BaseRenderer::glDebugOutput(GLenum source, GLenum type, GLuint id,
  GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
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

GLuint ansimproj::core::BaseRenderer::createVertFragShader(
  const std::vector<char> &vertSource, const std::vector<char> &fragSource) const {
  GLuint handle = glCreateProgram();
  if (!handle) {
    throw std::runtime_error("Unable to create shader program.");
  }

  GLuint vertHandle = glCreateShader(GL_VERTEX_SHADER);
  const GLint vertSize = vertSource.size();
  const char *vertShaderPtr = vertSource.data();
  glShaderSource(vertHandle, 1, &vertShaderPtr, &vertSize);
  glCompileShader(vertHandle);
  GLint logLength;
  GLint result = GL_FALSE;
  glGetShaderiv(vertHandle, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    glGetShaderiv(vertHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      std::vector<char> errorMessage(logLength + 1);
      glGetShaderInfoLog(vertHandle, logLength, nullptr, &errorMessage.front());
      std::string message(errorMessage.begin(), errorMessage.end());
      throw std::runtime_error("Unable to compile shader: " + message);
    } else {
      throw std::runtime_error("Unable to compile shader.");
    }
  }

  GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
  const GLint fragSize = fragSource.size();
  const char *fragShaderPtr = fragSource.data();
  glShaderSource(fragHandle, 1, &fragShaderPtr, &fragSize);
  glCompileShader(fragHandle);
  glGetShaderiv(fragHandle, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    glGetShaderiv(fragHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      std::vector<char> errorMessage(logLength + 1);
      glGetShaderInfoLog(fragHandle, logLength, nullptr, &errorMessage.front());
      std::string message(errorMessage.begin(), errorMessage.end());
      throw std::runtime_error("Unable to compile shader: " + message);
    } else {
      throw std::runtime_error("Unable to compile shader.");
    }
  }

  glAttachShader(handle, vertHandle);
  glAttachShader(handle, fragHandle);
  glLinkProgram(handle);
  glGetProgramiv(handle, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      std::vector<char> errorMessage(logLength + 1);
      glGetProgramInfoLog(handle, logLength, nullptr, &errorMessage.front());
      std::string message(errorMessage.begin(), errorMessage.end());
      throw std::runtime_error("Unable to link program: " + message);
    } else {
      throw std::runtime_error("Unable to link program.");
    }
  }

  glDetachShader(handle, vertHandle);
  glDetachShader(handle, fragHandle);
  glDeleteShader(vertHandle);
  glDeleteShader(fragHandle);
  return handle;
}

void ansimproj::core::BaseRenderer::deleteShader(const GLuint &handle) const {
  glDeleteProgram(handle);
}

GLuint ansimproj::core::BaseRenderer::createComputeShader(
  const std::vector<char> &shaderSource) const {
  GLuint handle = glCreateProgram();
  if (!handle) {
    throw std::runtime_error("Unable to create shader program.");
  }

  GLuint sourceHandle = glCreateShader(GL_COMPUTE_SHADER);
  const GLint sourceSize = shaderSource.size();
  const char *sourceShaderPtr = shaderSource.data();
  glShaderSource(sourceHandle, 1, &sourceShaderPtr, &sourceSize);
  glCompileShader(sourceHandle);
  GLint logLength;
  GLint result = GL_FALSE;
  glGetShaderiv(sourceHandle, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    glGetShaderiv(sourceHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      std::vector<char> errorMessage(logLength + 1);
      glGetShaderInfoLog(sourceHandle, logLength, nullptr, &errorMessage.front());
      std::string message(errorMessage.begin(), errorMessage.end());
      throw std::runtime_error("Unable to compile shader: " + message);
    } else {
      throw std::runtime_error("Unable to compile shader.");
    }
  }

  glAttachShader(handle, sourceHandle);
  glLinkProgram(handle);
  glGetProgramiv(handle, GL_LINK_STATUS, &result);
  if (result == GL_FALSE) {
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
      std::vector<char> errorMessage(logLength + 1);
      glGetProgramInfoLog(handle, logLength, nullptr, &errorMessage.front());
      std::string message(errorMessage.begin(), errorMessage.end());
      throw std::runtime_error("Unable to link program: " + message);
    } else {
      throw std::runtime_error("Unable to link program.");
    }
  }

  glDetachShader(handle, sourceHandle);
  glDeleteShader(sourceHandle);
  return handle;
}

GLuint ansimproj::core::BaseRenderer::createBuffer(
  const std::vector<float> &data, bool dynamic) const {
  GLuint handle;
  glCreateBuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  const auto size = data.size();
  glNamedBufferData(
    handle, size * sizeof(float), data.data(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  return handle;
}

GLuint ansimproj::core::BaseRenderer::createBuffer(
  const std::vector<GLuint> &data, bool dynamic) const {
  GLuint handle;
  glCreateBuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  const auto size = data.size();
  glNamedBufferData(
    handle, size * sizeof(GLuint), data.data(), dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  return handle;
}

void ansimproj::core::BaseRenderer::deleteBuffer(const GLuint &handle) const {
  glDeleteBuffers(1, &handle);
}

GLuint ansimproj::core::BaseRenderer::create1DTexture(std::uint32_t width, GLenum internalFormat,
  GLenum format, GLenum type, const std::vector<float> &data) const {
  GLuint handle;
  glCreateTextures(GL_TEXTURE_1D, 1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to 1D texture.");
  }
  glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  constexpr GLuint levels = 1;
  glTextureStorage1D(handle, levels, internalFormat, width);
  constexpr GLuint level = 0;
  constexpr GLuint offsetX = 0;
  glTextureSubImage1D(handle, level, offsetX, width, format, type, data.data());
  return handle;
}

GLuint ansimproj::core::BaseRenderer::create3DTexture(std::uint32_t width, std::uint32_t height,
  std::uint32_t depth, GLenum internalFormat, GLenum format, GLenum type,
  const std::vector<float> &data) const {
  GLuint handle;
  glCreateTextures(GL_TEXTURE_3D, 1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to 3D texture.");
  }
  glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTextureParameteri(handle, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
  constexpr GLuint levels = 1;
  glTextureStorage3D(handle, levels, internalFormat, width, height, depth);
  constexpr GLuint level = 0;
  constexpr GLuint offsetX = 0;
  constexpr GLuint offsetY = 0;
  constexpr GLuint offsetZ = 0;
  glTextureSubImage3D(
    handle, level, offsetX, offsetY, offsetZ, width, height, depth, format, type, data.data());
  return handle;
}

void ansimproj::core::BaseRenderer::deleteTexture(const GLuint &handle) const {
  glDeleteTextures(1, &handle);
}
