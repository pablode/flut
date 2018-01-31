#include "BaseRenderer.hpp"

#include <iostream>

ansimproj::core::BaseRenderer::BaseRenderer() {
  glGetIntegerv(GL_MAJOR_VERSION, &versionMajor_);
  glGetIntegerv(GL_MINOR_VERSION, &versionMinor_);

#ifndef NDEBUG
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
#endif

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
  glGenBuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  const auto size = data.size();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(float), data.data(),
    dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  return handle;
}

GLuint ansimproj::core::BaseRenderer::createBuffer(
  const std::vector<GLuint> &data, bool dynamic) const {
  GLuint handle;
  glGenBuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  const auto size = data.size();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size * sizeof(GLuint), data.data(),
    dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  return handle;
}

GLuint ansimproj::core::BaseRenderer::createFullFBO(
  const GLuint &depthTexture, std::vector<GLuint> colorTextures) const {
  GLuint handle;
  glGenFramebuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, handle);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
  for (std::uint32_t i = 0; i < colorTextures.size(); ++i)
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorTextures[i], 0);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return handle;
  } else {
    switch (status) {
    case GL_FRAMEBUFFER_UNDEFINED:
      throw std::runtime_error("Default Framebuffer does not exist!");
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      throw std::runtime_error("Invalid attachment point(s)!");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      throw std::runtime_error("Framebuffer does not have image attached!");
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      throw std::runtime_error(
        "Value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment "
        "point(s) named by GL_DRAW_BUFFERi!");
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      throw std::runtime_error(
        "GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
        "is GL_NONE for the color attachment point named by GL_READ_BUFFER.");
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      throw std::runtime_error(
        "Value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers or "
        "value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures");
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      throw std::runtime_error(
        "Any framebuffer attachment is layered, and any populated attachment is not layered, or "
        "all populated color attachments are not from textures of the same target.");
    case GL_FRAMEBUFFER_UNSUPPORTED:
      throw std::runtime_error(
        "The combination of internal formats of the attached images violates an "
        "implementation-dependent set of restrictions");
    default:
      throw std::runtime_error("Unknown Framebuffer error code.");
    }
  }
}

GLuint ansimproj::core::BaseRenderer::createFlatFBO(const GLuint &colorTexture) const {
  GLuint handle;
  glGenFramebuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, handle);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return handle;
  } else {
    switch (status) {
    case GL_FRAMEBUFFER_UNDEFINED:
      throw std::runtime_error("Default Framebuffer does not exist!");
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      throw std::runtime_error("Invalid attachment point(s)!");
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      throw std::runtime_error("Framebuffer does not have image attached!");
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      throw std::runtime_error(
        "Value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment "
        "point(s) named by GL_DRAW_BUFFERi!");
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      throw std::runtime_error(
        "GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
        "is GL_NONE for the color attachment point named by GL_READ_BUFFER.");
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      throw std::runtime_error(
        "Value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers or "
        "value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures");
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      throw std::runtime_error(
        "Any framebuffer attachment is layered, and any populated attachment is not layered, or "
        "all populated color attachments are not from textures of the same target.");
    case GL_FRAMEBUFFER_UNSUPPORTED:
      throw std::runtime_error(
        "The combination of internal formats of the attached images violates an "
        "implementation-dependent set of restrictions");
    default:
      throw std::runtime_error("Unknown Framebuffer error code.");
    }
  }
}

GLuint ansimproj::core::BaseRenderer::createColorTexture(
  const std::uint32_t &width, const std::uint32_t &height) const {
  GLuint handle;
  glGenTextures(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create texture.");
  }
  glBindTexture(GL_TEXTURE_2D, handle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return handle;
}

GLuint ansimproj::core::BaseRenderer::createDepthTexture(
  const std::uint32_t &width, const std::uint32_t &height) const {
  GLuint handle;
  glGenTextures(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create texture.");
  }
  glBindTexture(GL_TEXTURE_2D, handle);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT,
    GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return handle;
}

void ansimproj::core::BaseRenderer::deleteTexture(const GLuint &handle) const {
  glDeleteTextures(1, &handle);
}

void ansimproj::core::BaseRenderer::deleteFBO(const GLuint &handle) const {
  glDeleteFramebuffers(1, &handle);
}

void ansimproj::core::BaseRenderer::deleteBuffer(const GLuint &handle) const {
  glDeleteBuffers(1, &handle);
}
