#include "SimulationBase.hpp"

#include <glad/glad.h>
#include <iostream>

using namespace flut;
using namespace flut::core;

SimulationBase::SimulationBase()
{
  glGetIntegerv(GL_MAJOR_VERSION, &versionMajor_);
  glGetIntegerv(GL_MINOR_VERSION, &versionMinor_);

#ifndef NDEBUG
  GLint flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != GL_NONE)
  {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback((GLDEBUGPROC)&glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    std::printf("Debug output enabled.\n");
  }
  else
  {
    std::printf("Debug output not available (context flag not set).\n");
  }

  glad_set_post_callback((GLADcallback)[](const char* name, void* funcptr, int len_args, ...) {
    if (!strcmp(name, "glGetError")) {
      return;
    }
    GLenum errorCode = glGetError();
    if (errorCode != GL_NO_ERROR) {
      do {
        std::printf("GL/ERROR: 0x%04x in %s\n", errorCode, name);
      } while ((errorCode = glGetError()) != GL_NO_ERROR);
    }
  });
#endif

  if (!GLAD_GL_ARB_bindless_texture)
  {
    throw std::runtime_error{"ARB_bindless_texture extension is required."};
  }

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
}

SimulationBase::~SimulationBase() {}

void SimulationBase::glDebugOutput(
  GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  const char* sourceStr = "Unknown";
  if (source == GL_DEBUG_SOURCE_API) {
    sourceStr = "API";
  }
  else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM) {
    sourceStr = "Window System";
  }
  else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER) {
    sourceStr = "Shader Compiler";
  }
  else if (source == GL_DEBUG_SOURCE_THIRD_PARTY) {
    sourceStr = "Third Party";
  }
  else if (source == GL_DEBUG_SOURCE_APPLICATION) {
    sourceStr = "Application";
  }
  else if (source == GL_DEBUG_SOURCE_OTHER) {
    sourceStr = "Other";
  }

  const char* typeStr = "Unknown";
  if (type == GL_DEBUG_TYPE_ERROR) {
    typeStr = "Error";
  }
  else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) {
    typeStr = "Deprecated Behaviour";
  }
  else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) {
    typeStr = "Undefined Behaviour";
  }
  else if (type == GL_DEBUG_TYPE_PORTABILITY) {
    typeStr = "Portability";
  }
  else if (type == GL_DEBUG_TYPE_PERFORMANCE) {
    typeStr = "Performance";
  }
  else if (type == GL_DEBUG_TYPE_MARKER) {
    typeStr = "Marker";
  }
  else if (type == GL_DEBUG_TYPE_PUSH_GROUP) {
    typeStr = "Push Group";
  }
  else if (type == GL_DEBUG_TYPE_POP_GROUP) {
    typeStr = "Pop Group";
  }
  else if (type == GL_DEBUG_TYPE_OTHER) {
    typeStr = "Other";
  }

  if (severity == GL_DEBUG_SEVERITY_HIGH) {
    std::printf("GL/ERROR: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
  }
  else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
    std::printf("GL/WARNING: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
  }
  else if (severity == GL_DEBUG_SEVERITY_LOW) {
    std::printf("GL/INFO: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
  }
  else {
    std::printf("GL/DEBUG: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
  }
}

GLuint SimulationBase::createVertFragShader(
  const std::vector<char>& vertSource, const std::vector<char>& fragSource) const
{
  GLuint handle = glCreateProgram();
  if (!handle) {
    throw std::runtime_error("Unable to create shader program.");
  }

  const GLuint vertHandle = glCreateShader(GL_VERTEX_SHADER);
  const GLint vertSize = vertSource.size();
  const char* vertShaderPtr = vertSource.data();
  glShaderSource(vertHandle, 1, &vertShaderPtr, &vertSize);
  glCompileShader(vertHandle);
  GLint logLength;
  GLint result = GL_FALSE;
  glGetShaderiv(vertHandle, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetShaderiv(vertHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      throw std::runtime_error("Unable to compile shader.");
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetShaderInfoLog(vertHandle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    throw std::runtime_error("Unable to compile shader: " + message);
  }

  const GLuint fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
  const GLint fragSize = fragSource.size();
  const char* fragShaderPtr = fragSource.data();
  glShaderSource(fragHandle, 1, &fragShaderPtr, &fragSize);
  glCompileShader(fragHandle);
  glGetShaderiv(fragHandle, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetShaderiv(fragHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      throw std::runtime_error("Unable to compile shader.");
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetShaderInfoLog(fragHandle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    throw std::runtime_error("Unable to compile shader: " + message);
  }

  glAttachShader(handle, vertHandle);
  glAttachShader(handle, fragHandle);
  glLinkProgram(handle);
  glGetProgramiv(handle, GL_LINK_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      throw std::runtime_error("Unable to link program.");
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetProgramInfoLog(handle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    throw std::runtime_error("Unable to link program: " + message);
  }

  glDetachShader(handle, vertHandle);
  glDetachShader(handle, fragHandle);
  glDeleteShader(vertHandle);
  glDeleteShader(fragHandle);
  return handle;
}

void SimulationBase::deleteShader(GLuint handle) const
{
  glDeleteProgram(handle);
}

GLuint SimulationBase::createComputeShader(const std::vector<char>& shaderSource) const
{
  const GLuint handle = glCreateProgram();
  if (!handle) {
    throw std::runtime_error("Unable to create shader program.");
  }

  const GLuint sourceHandle = glCreateShader(GL_COMPUTE_SHADER);
  const GLint sourceSize = shaderSource.size();
  const char* sourceShaderPtr = shaderSource.data();
  glShaderSource(sourceHandle, 1, &sourceShaderPtr, &sourceSize);
  glCompileShader(sourceHandle);
  GLint logLength;
  GLint result = GL_FALSE;
  glGetShaderiv(sourceHandle, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetShaderiv(sourceHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      throw std::runtime_error("Unable to compile shader.");
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetShaderInfoLog(sourceHandle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    throw std::runtime_error("Unable to compile shader: " + message);
  }

  glAttachShader(handle, sourceHandle);
  glLinkProgram(handle);
  glGetProgramiv(handle, GL_LINK_STATUS, &result);
  if (result == GL_FALSE)
  {
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      throw std::runtime_error("Unable to link program.");
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetProgramInfoLog(handle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    throw std::runtime_error("Unable to link program: " + message);
  }

  glDetachShader(handle, sourceHandle);
  glDeleteShader(sourceHandle);
  return handle;
}

GLuint SimulationBase::createBuffer(const std::vector<float>& data, bool dynamic) const
{
  GLuint handle;
  glCreateBuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  const auto size = data.size() * sizeof(float);
  glNamedBufferStorage(handle, size, data.data(), dynamic ? GL_DYNAMIC_STORAGE_BIT : 0);
  return handle;
}

GLuint SimulationBase::createBuffer(const std::vector<GLuint>& data, bool dynamic) const
{
  GLuint handle;
  glCreateBuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }
  const auto size = data.size() * sizeof(float);
  glNamedBufferStorage(handle, size, data.data(), dynamic ? GL_DYNAMIC_STORAGE_BIT : 0);
  return handle;
}

GLuint SimulationBase::createFullFBO(GLuint depthTexture, std::vector<GLuint> colorTextures) const
{
  GLuint handle;
  glCreateFramebuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }

  glNamedFramebufferTexture(handle, GL_DEPTH_ATTACHMENT, depthTexture, 0);
  for (std::uint32_t i = 0; i < colorTextures.size(); ++i) {
    glNamedFramebufferTexture(handle, GL_COLOR_ATTACHMENT0 + i, colorTextures[i], 0);
  }

  const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return handle;
  }

  switch (status)
  {
  case GL_FRAMEBUFFER_UNDEFINED:
    throw std::runtime_error("Default Framebuffer does not exist!");
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    throw std::runtime_error("Invalid attachment point(s)!");
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    throw std::runtime_error("Framebuffer does not have image attached!");
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    throw std::runtime_error("Value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment "
                             "point(s) named by GL_DRAW_BUFFERi!");
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    throw std::runtime_error("GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
                             "is GL_NONE for the color attachment point named by GL_READ_BUFFER.");
  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    throw std::runtime_error("Value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers or "
                             "value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures");
  case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    throw std::runtime_error("Any framebuffer attachment is layered, and any populated attachment is not layered, or "
                             "all populated color attachments are not from textures of the same target.");
  case GL_FRAMEBUFFER_UNSUPPORTED:
    throw std::runtime_error("The combination of internal formats of the attached images violates an "
                             "implementation-dependent set of restrictions");
  default:
    throw std::runtime_error("Unknown Framebuffer error code.");
  }
}

GLuint SimulationBase::createFlatFBO(GLuint colorTexture) const
{
  GLuint handle;
  glCreateFramebuffers(1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create buffer.");
  }

  glNamedFramebufferTexture(handle, GL_COLOR_ATTACHMENT0, colorTexture, 0);

  const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return handle;
  }

  switch (status)
  {
  case GL_FRAMEBUFFER_UNDEFINED:
    throw std::runtime_error("Default Framebuffer does not exist!");
  case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    throw std::runtime_error("Invalid attachment point(s)!");
  case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    throw std::runtime_error("Framebuffer does not have image attached!");
  case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    throw std::runtime_error("Value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any color attachment "
                             "point(s) named by GL_DRAW_BUFFERi!");
  case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    throw std::runtime_error("GL_READ_BUFFER is not GL_NONE and the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE "
                             "is GL_NONE for the color attachment point named by GL_READ_BUFFER.");
  case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
    throw std::runtime_error("Value of GL_RENDERBUFFER_SAMPLES is not the same for all attached renderbuffers or "
                             "value of GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not the same for all attached textures");
  case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
    throw std::runtime_error("Any framebuffer attachment is layered, and any populated attachment is not layered, or "
                             "all populated color attachments are not from textures of the same target.");
  case GL_FRAMEBUFFER_UNSUPPORTED:
    throw std::runtime_error("The combination of internal formats of the attached images violates an "
                             "implementation-dependent set of restrictions");
  default:
    throw std::runtime_error("Unknown Framebuffer error code.");
  }
}

GLuint SimulationBase::createR32FColorTexture(std::uint32_t width, std::uint32_t height) const
{
  return createColorTexture(GL_R32F, width, height);
}

GLuint SimulationBase::createRGB32FColorTexture(std::uint32_t width, std::uint32_t height) const
{
  return createColorTexture(GL_RGB32F, width, height);
}

GLuint SimulationBase::createColorTexture(GLenum internalFormat, std::uint32_t width, std::uint32_t height) const
{
  GLuint handle;
  glCreateTextures(GL_TEXTURE_2D, 1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create texture.");
  }
  glTextureStorage2D(handle, 1, internalFormat, width, height);
  glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return handle;
}

GLuint SimulationBase::createDepthTexture(std::uint32_t width, std::uint32_t height) const
{
  GLuint handle;
  glCreateTextures(GL_TEXTURE_2D, 1, &handle);
  if (!handle) {
    throw std::runtime_error("Unable to create texture.");
  }
  glTextureStorage2D(handle, 1, GL_DEPTH_COMPONENT24, width, height);
  glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return handle;
}

GLuint64 SimulationBase::makeTextureResident(GLuint handle) const
{
  GLuint64 bindlessHandle = glGetTextureHandleARB(handle);
  glMakeTextureHandleResidentARB(bindlessHandle);
  return bindlessHandle;
}

void SimulationBase::makeTextureNonResident(GLuint64 bindlessHandle) const
{
  glMakeTextureHandleNonResidentARB(bindlessHandle);
}

void SimulationBase::deleteTexture(GLuint handle) const
{
  glDeleteTextures(1, &handle);
}

void SimulationBase::deleteFBO(GLuint handle) const
{
  glDeleteFramebuffers(1, &handle);
}

void SimulationBase::deleteBuffer(GLuint handle) const
{
  glDeleteBuffers(1, &handle);
}
