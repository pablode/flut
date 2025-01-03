#include "GlHelper.hpp"

#include <sstream>
#include <fstream>

using namespace flut;

void _gladPostCallback(char* name, void* funcptr, int len_args, ...)
{
  if (!strcmp(name, "glGetError")) {
    return;
  }
  GLenum errorCode = glGetError();
  if (errorCode != GL_NO_ERROR) {
    do {
      fprintf(stderr, "GL/ERROR: 0x%04x in %s\n", errorCode, name);
    } while ((errorCode = glGetError()) != GL_NO_ERROR);
  }
}

void GlHelper::enableDebugHooks()
{
  GLint flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if ((flags & GL_CONTEXT_FLAG_DEBUG_BIT) != GL_NONE)
  {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback((GLDEBUGPROC) &glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    printf("Debug output enabled.\n");
  }
  else
  {
    printf("Debug output not available (context flag not set).\n");
  }
  fflush(stdout);

  glad_set_post_callback((GLADcallback) _gladPostCallback);
}

std::string GlHelper::loadFileText(const std::string& filePath)
{
  std::ifstream file{ filePath, std::ios_base::in | std::ios_base::binary };
  if (!file.is_open()) {
    fprintf(stderr, "Unable to open file %s", filePath.c_str());
    abort();
  }

  std::string text;
  file.seekg(0, std::ios_base::end);
  text.resize(file.tellg());
  file.seekg(0, std::ios_base::beg);
  text.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  return text;
}

std::string GlHelper::preprocessShaderSource(const std::string& text, std::vector<GlHelper::ShaderDefine> defines)
{
  std::stringstream ss;

  ss << "#version 460 core\n";
  ss << "#extension GL_ARB_bindless_texture : require\n";

  for (auto& define : defines)
  {
    ss << "#define " << define.name << " " << define.valueStr << "\n";
  }

  ss << text;

  return ss.str();
}

GLuint GlHelper::createVertFragShader(const char* vertPath, const char* fragPath, std::vector<ShaderDefine> defines)
{
  GLuint handle = glCreateProgram();

  if (!handle) {
    fprintf(stderr, "Unable to create shader program.");
    abort();
  }

  std::string vertSource = loadFileText(vertPath);
  vertSource = preprocessShaderSource(vertSource, defines);

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
      fprintf(stderr, "Unable to compile shader");
      abort();
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetShaderInfoLog(vertHandle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    fprintf(stderr, "Unable to compile shader: %s", message.c_str());
    abort();
  }

  std::string fragSource = loadFileText(fragPath);
  fragSource = preprocessShaderSource(fragSource, defines);

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
      fprintf(stderr, "Unable to compile shader");
      abort();
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetShaderInfoLog(fragHandle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    fprintf(stderr, "Unable to compile shader: %s", message.c_str());
    abort();
  }

  glAttachShader(handle, vertHandle);
  glAttachShader(handle, fragHandle);
  glLinkProgram(handle);

  glGetProgramiv(handle, GL_LINK_STATUS, &result);

  if (result == GL_FALSE)
  {
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      fprintf(stderr, "Unable to link program");
      abort();
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetProgramInfoLog(handle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    fprintf(stderr, "Unable to link program: %s", message.c_str());
    abort();
  }

  glDetachShader(handle, vertHandle);
  glDetachShader(handle, fragHandle);
  glDeleteShader(vertHandle);
  glDeleteShader(fragHandle);
  return handle;
}

GLuint GlHelper::createComputeShader(const char* path, std::vector<ShaderDefine> defines)
{
  const GLuint handle = glCreateProgram();

  if (!handle) {
    throw std::runtime_error("Unable to create shader program.");
  }

  std::string source = loadFileText(path);
  source = preprocessShaderSource(source, defines);

  const GLuint sourceHandle = glCreateShader(GL_COMPUTE_SHADER);
  const GLint sourceSize = source.size();
  const char* sourceShaderPtr = source.data();
  glShaderSource(sourceHandle, 1, &sourceShaderPtr, &sourceSize);
  glCompileShader(sourceHandle);

  GLint logLength;
  GLint result = GL_FALSE;
  glGetShaderiv(sourceHandle, GL_COMPILE_STATUS, &result);

  if (result == GL_FALSE)
  {
    glGetShaderiv(sourceHandle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      fprintf(stderr, "Unable to compile shader");
      abort();
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetShaderInfoLog(sourceHandle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    fprintf(stderr, "Unable to compile shader: %s", message.c_str());
    abort();
  }

  glAttachShader(handle, sourceHandle);
  glLinkProgram(handle);

  glGetProgramiv(handle, GL_LINK_STATUS, &result);

  if (result == GL_FALSE)
  {
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength == 0) {
      fprintf(stderr, "Unable to link program");
      abort();
    }
    std::vector<char> errorMessage(logLength + 1);
    glGetProgramInfoLog(handle, logLength, nullptr, &errorMessage.front());
    std::string message(errorMessage.begin(), errorMessage.end());
    fprintf(stderr, "Unable to link program: %s", message.c_str());
    abort();
  }

  glDetachShader(handle, sourceHandle);
  glDeleteShader(sourceHandle);
  return handle;
}

void GlHelper::glDebugOutput(
  GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
  const char* sourceStr = "Unknown";
  if (source == GL_DEBUG_SOURCE_API) { sourceStr = "API"; }
  else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM) { sourceStr = "Window System"; }
  else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER) { sourceStr = "Shader Compiler"; }
  else if (source == GL_DEBUG_SOURCE_THIRD_PARTY) { sourceStr = "Third Party"; }
  else if (source == GL_DEBUG_SOURCE_APPLICATION) { sourceStr = "Application"; }
  else if (source == GL_DEBUG_SOURCE_OTHER) { sourceStr = "Other"; }

  const char* typeStr = "Unknown";
  if (type == GL_DEBUG_TYPE_ERROR) { typeStr = "Error"; }
  else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR) { typeStr = "Deprecated Behaviour"; }
  else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR) { typeStr = "Undefined Behaviour"; }
  else if (type == GL_DEBUG_TYPE_PORTABILITY) { typeStr = "Portability"; }
  else if (type == GL_DEBUG_TYPE_PERFORMANCE) { typeStr = "Performance"; }
  else if (type == GL_DEBUG_TYPE_MARKER) { typeStr = "Marker"; }
  else if (type == GL_DEBUG_TYPE_PUSH_GROUP) { typeStr = "Push Group"; }
  else if (type == GL_DEBUG_TYPE_POP_GROUP) { typeStr = "Pop Group"; }
  else if (type == GL_DEBUG_TYPE_OTHER) { typeStr = "Other"; }

  if (severity == GL_DEBUG_SEVERITY_HIGH) {
    fprintf(stderr, "GL/ERROR: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
  }
  else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
    fprintf(stderr, "GL/WARNING: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
  }
  else if (severity == GL_DEBUG_SEVERITY_LOW) {
    printf("GL/INFO: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
    fflush(stdout);
  }
  else {
    printf("GL/DEBUG: \"%s\" (%s/%s)\n", message, sourceStr, typeStr);
    fflush(stdout);
  }
}
