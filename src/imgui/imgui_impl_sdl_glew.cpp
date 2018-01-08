//
// ImGui SDL2 binding with GLEW.
// Based on OpenGL3 features and C++11.
//
// In this binding, ImTextureID is used to store an OpenGL 'GLuint' texture identifier. Read the FAQ
// about ImTextureID in imgui.cpp.
// SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan
// graphics context creation, etc. GLEW is a helper library to access OpenGL functions since
// there is no standard header to access modern OpenGL functions easily. Alternatives are GL3W,
// glbinding, Glad, etc.
//
// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example
// of using this. If you use this binding you'll need to call 4 functions: ImGui_ImplXXXX_Init(),
// ImGui_ImplXXXX_NewFrame(), ImGui::Render() and ImGui_ImplXXXX_Shutdown(). If you are new to
// ImGui, see examples/README.txt and documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui
//

#include "imgui.h"
#include "imgui_impl_sdl_glew.h"

#include <cstdint>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
// Be sure to call "glewInit()"
// somewhere between context creation and invoking this implementation.

///
static double time_ = 0.0f;
static bool mousePressed_[3] = {false, false, false};
static float mouseWheel_ = 0.0f;
static GLuint fontTexture_ = 0;
static GLuint shaderHandle_ = 0, vertHandle_ = 0, fragHandle_ = 0;
static GLint attribLocationTex_ = 0, attribLocationProj_ = 0, attribLocationPos_ = 0,
             attribLocationUV_ = 0, attribLocationCol_ = 0;
static GLuint vboHandle_ = 0, vaoHandle_ = 0, elementsHandle_ = 0;

///
void ImGui_ImplSdlGlew_RenderDrawLists(ImDrawData *drawData) {
  // Avoid rendering when minimized, scale coordinates for retina displays
  ImGuiIO &io = ImGui::GetIO();
  int framebufWidth = static_cast<std::uint32_t>(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int framebufHeight = static_cast<std::uint32_t>(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if (framebufWidth == 0 || framebufHeight == 0)
    return;
  drawData->ScaleClipRects(io.DisplayFramebufferScale);

  // Back up GL state
  GLenum lastActiveTexture;
  glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *)&lastActiveTexture);
  glActiveTexture(GL_TEXTURE0);
  GLint lastProgram;
  glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
  GLint lastTexture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
  GLint lastSampler;
  glGetIntegerv(GL_SAMPLER_BINDING, &lastSampler);
  GLint lastArrayBuffer;
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
  GLint lastElementArrayBuffer;
  glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &lastElementArrayBuffer);
  GLint lastVertexArray;
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArray);
  GLint lastPolygonMode[2];
  glGetIntegerv(GL_POLYGON_MODE, lastPolygonMode);
  GLint lastViewport[4];
  glGetIntegerv(GL_VIEWPORT, lastViewport);
  GLint lastScissorBox[4];
  glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox);
  GLenum lastBlendSrcRgb;
  glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *)&lastBlendSrcRgb);
  GLenum lastBlendDstRgb;
  glGetIntegerv(GL_BLEND_DST_RGB, (GLint *)&lastBlendDstRgb);
  GLenum lastBlendSrcAlpha;
  glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *)&lastBlendSrcAlpha);
  GLenum lastBlendDstAlpha;
  glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *)&lastBlendDstAlpha);
  GLenum lastBlendEquationRgb;
  glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint *)&lastBlendEquationRgb);
  GLenum lastBlendEquationAlpha;
  glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint *)&lastBlendEquationAlpha);
  GLboolean lastEnableBlend = glIsEnabled(GL_BLEND);
  GLboolean lastEnableCullface = glIsEnabled(GL_CULL_FACE);
  GLboolean lastEnableDepthTest = glIsEnabled(GL_DEPTH_TEST);
  GLboolean lastEnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);

  // Setup render state
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Setup viewport, orthographic projection matrix
  glViewport(0, 0, (GLsizei)framebufWidth, (GLsizei)framebufHeight);
  const float orthoProj[4][4] = {
    {2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f},
    {0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f},
    {0.0f, 0.0f, -1.0f, 0.0f},
    {-1.0f, 1.0f, 0.0f, 1.0f},
  };
  glUseProgram(shaderHandle_);
  glUniform1i(attribLocationTex_, 0);
  glUniformMatrix4fv(attribLocationProj_, 1, GL_FALSE, &orthoProj[0][0]);
  glBindVertexArray(vaoHandle_);
  // Rely on combined texture/sampler state.
  glBindSampler(0, 0);

  for (int n = 0; n < drawData->CmdListsCount; n++) {
    const ImDrawList *cmdList = drawData->CmdLists[n];
    const ImDrawIdx *bufferOffset = 0;

    glBindBuffer(GL_ARRAY_BUFFER, vboHandle_);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmdList->VtxBuffer.Size * sizeof(ImDrawVert),
      (const GLvoid *)cmdList->VtxBuffer.Data, GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsHandle_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmdList->IdxBuffer.Size * sizeof(ImDrawIdx),
      (const GLvoid *)cmdList->IdxBuffer.Data, GL_STREAM_DRAW);

    for (int i = 0; i < cmdList->CmdBuffer.Size; i++) {
      const ImDrawCmd *pcmd = &cmdList->CmdBuffer[i];
      if (pcmd->UserCallback) {
        pcmd->UserCallback(cmdList, pcmd);
      } else {
        glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
        glScissor((int)pcmd->ClipRect.x, (int)(framebufHeight - pcmd->ClipRect.w),
          (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
        glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount,
          sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, bufferOffset);
      }
      bufferOffset += pcmd->ElemCount;
    }
  }

  // Restore modified GL state
  glUseProgram(static_cast<GLuint>(lastProgram));
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(lastTexture));
  glBindSampler(0, static_cast<GLuint>(lastSampler));
  glActiveTexture(lastActiveTexture);
  glBindVertexArray(static_cast<GLuint>(lastVertexArray));
  glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(lastArrayBuffer));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(lastElementArrayBuffer));
  glBlendEquationSeparate(lastBlendEquationRgb, lastBlendEquationAlpha);
  glBlendFuncSeparate(lastBlendSrcRgb, lastBlendDstRgb, lastBlendSrcAlpha, lastBlendDstAlpha);
  if (lastEnableBlend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);
  if (lastEnableCullface)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);
  if (lastEnableDepthTest)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
  if (lastEnableScissorTest)
    glEnable(GL_SCISSOR_TEST);
  else
    glDisable(GL_SCISSOR_TEST);
  glPolygonMode(GL_FRONT_AND_BACK, (GLenum)(lastPolygonMode[0]));
  glViewport(lastViewport[0], lastViewport[1], (GLsizei)lastViewport[2], (GLsizei)lastViewport[3]);
  glScissor(
    lastScissorBox[0], lastScissorBox[1], (GLsizei)lastScissorBox[2], (GLsizei)lastScissorBox[3]);
}

static const char *ImGui_ImplSdlGlew_GetClipboardText(void *) {
  return SDL_GetClipboardText();
}

static void ImGui_ImplSdlGlew_SetClipboardText(void *, const char *text) {
  SDL_SetClipboardText(text);
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to
// use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
//   application. Generally you may always pass all inputs to dear imgui, and hide them from your
//   application based on those two flags.
bool ImGui_ImplSdlGlew_ProcessEvent(SDL_Event *event) {
  ImGuiIO &io = ImGui::GetIO();
  switch (event->type) {
  case SDL_MOUSEWHEEL: {
    if (event->wheel.y > 0)
      mouseWheel_ = 1;
    if (event->wheel.y < 0)
      mouseWheel_ = -1;
    return true;
  }
  case SDL_MOUSEBUTTONDOWN: {
    if (event->button.button == SDL_BUTTON_LEFT)
      mousePressed_[0] = true;
    if (event->button.button == SDL_BUTTON_RIGHT)
      mousePressed_[1] = true;
    if (event->button.button == SDL_BUTTON_MIDDLE)
      mousePressed_[2] = true;
    return true;
  }
  case SDL_TEXTINPUT: {
    io.AddInputCharactersUTF8(event->text.text);
    return true;
  }
  case SDL_KEYDOWN:
  case SDL_KEYUP: {
    int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
    io.KeysDown[key] = (event->type == SDL_KEYDOWN);
    io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
    io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
    io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
    io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
    return true;
  }
  default: { return false; }
  }
}

void ImGui_ImplSdlGlew_CreateFontsTexture() {
  // Build texture atlas
  ImGuiIO &io = ImGui::GetIO();
  unsigned char *pixels;
  int width, height;
  // Load as RGBA 32-bits because it is more likely to be compatible with user's existing shader
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  // Upload texture
  GLint lastTexture;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
  glGenTextures(1, &fontTexture_);
  glBindTexture(GL_TEXTURE_2D, fontTexture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  // Store our identifier
  io.Fonts->TexID = (void *)(intptr_t)fontTexture_;
  // Restore state
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(lastTexture));
}

bool ImGui_ImplSdlGlew_CreateDeviceObjects() {
  // Backup GL state
  GLint lastTexture, lastArrayBuffer, lastVertexArray;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
  glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArray);

  constexpr const GLchar *vertexShader =
    "#version 330\n"
    "uniform mat4 Projection;\n"
    "in vec2 Position;\n"
    "in vec2 UV;\n"
    "in vec4 Color;\n"
    "out vec2 Frag_UV;\n"
    "out vec4 Frag_Color;\n"
    "void main()\n"
    "{\n"
    "	 Frag_UV = UV;\n"
    "	 Frag_Color = Color;\n"
    "	 gl_Position = Projection * vec4(Position.xy,0,1);\n"
    "}\n";

  constexpr const GLchar *fragmentShader =
    "#version 330\n"
    "uniform sampler2D Texture;\n"
    "in vec2 Frag_UV;\n"
    "in vec4 Frag_Color;\n"
    "out vec4 Out_Color;\n"
    "void main()\n"
    "{\n"
    "	 Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
    "}\n";

  shaderHandle_ = glCreateProgram();
  vertHandle_ = glCreateShader(GL_VERTEX_SHADER);
  fragHandle_ = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(vertHandle_, 1, &vertexShader, 0);
  glShaderSource(fragHandle_, 1, &fragmentShader, 0);
  glCompileShader(vertHandle_);
  glCompileShader(fragHandle_);
  glAttachShader(shaderHandle_, vertHandle_);
  glAttachShader(shaderHandle_, fragHandle_);
  glLinkProgram(shaderHandle_);

  attribLocationTex_ = glGetUniformLocation(shaderHandle_, "Texture");
  attribLocationProj_ = glGetUniformLocation(shaderHandle_, "Projection");
  attribLocationPos_ = glGetAttribLocation(shaderHandle_, "Position");
  attribLocationUV_ = glGetAttribLocation(shaderHandle_, "UV");
  attribLocationCol_ = glGetAttribLocation(shaderHandle_, "Color");

  glGenBuffers(1, &vboHandle_);
  glGenBuffers(1, &elementsHandle_);

  glGenVertexArrays(1, &vaoHandle_);
  glBindVertexArray(vaoHandle_);
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle_);
  glEnableVertexAttribArray(static_cast<GLuint>(attribLocationPos_));
  glEnableVertexAttribArray(static_cast<GLuint>(attribLocationUV_));
  glEnableVertexAttribArray(static_cast<GLuint>(attribLocationCol_));

#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))
  glVertexAttribPointer(static_cast<GLuint>(attribLocationPos_), 2, GL_FLOAT, GL_FALSE,
    sizeof(ImDrawVert), (GLvoid *)OFFSETOF(ImDrawVert, pos));
  glVertexAttribPointer(static_cast<GLuint>(attribLocationUV_), 2, GL_FLOAT, GL_FALSE,
    sizeof(ImDrawVert), (GLvoid *)OFFSETOF(ImDrawVert, uv));
  glVertexAttribPointer(static_cast<GLuint>(attribLocationCol_), 4, GL_UNSIGNED_BYTE, GL_TRUE,
    sizeof(ImDrawVert), (GLvoid *)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

  ImGui_ImplSdlGlew_CreateFontsTexture();

  // Restore modified GL state
  glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(lastTexture));
  glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(lastArrayBuffer));
  glBindVertexArray(static_cast<GLuint>(lastVertexArray));
  return true;
}

void ImGui_ImplSdlGlew_InvalidateDeviceObjects() {
  if (vaoHandle_)
    glDeleteVertexArrays(1, &vaoHandle_);
  if (vboHandle_)
    glDeleteBuffers(1, &vboHandle_);
  if (elementsHandle_)
    glDeleteBuffers(1, &elementsHandle_);
  vaoHandle_ = vboHandle_ = elementsHandle_ = 0;

  if (shaderHandle_ && vertHandle_)
    glDetachShader(shaderHandle_, vertHandle_);
  if (vertHandle_)
    glDeleteShader(vertHandle_);
  vertHandle_ = 0;

  if (shaderHandle_ && fragHandle_)
    glDetachShader(shaderHandle_, fragHandle_);
  if (fragHandle_)
    glDeleteShader(fragHandle_);
  fragHandle_ = 0;

  if (shaderHandle_)
    glDeleteProgram(shaderHandle_);
  shaderHandle_ = 0;

  if (fontTexture_) {
    glDeleteTextures(1, &fontTexture_);
    ImGui::GetIO().Fonts->TexID = 0;
    fontTexture_ = 0;
  }
}

bool ImGui_ImplSdlGlew_Init(SDL_Window *window) {
  ImGuiIO &io = ImGui::GetIO();
  io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
  io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
  io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
  io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
  io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
  io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
  io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
  io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
  io.KeyMap[ImGuiKey_A] = SDLK_a;
  io.KeyMap[ImGuiKey_C] = SDLK_c;
  io.KeyMap[ImGuiKey_V] = SDLK_v;
  io.KeyMap[ImGuiKey_X] = SDLK_x;
  io.KeyMap[ImGuiKey_Y] = SDLK_y;
  io.KeyMap[ImGuiKey_Z] = SDLK_z;

  io.RenderDrawListsFn = ImGui_ImplSdlGlew_RenderDrawLists;
  io.SetClipboardTextFn = ImGui_ImplSdlGlew_SetClipboardText;
  io.GetClipboardTextFn = ImGui_ImplSdlGlew_GetClipboardText;
  io.ClipboardUserData = nullptr;

#ifdef _WIN32
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInfo(window, &wmInfo);
  io.ImeWindowHandle = wmInfo.info.win.window;
#else
  (void)window;
#endif
  return true;
}

void ImGui_ImplSdlGlew_Shutdown() {
  ImGui_ImplSdlGlew_InvalidateDeviceObjects();
  ImGui::Shutdown();
}

void ImGui_ImplSdlGlew_NewFrame(SDL_Window *window) {
  if (!fontTexture_)
    ImGui_ImplSdlGlew_CreateDeviceObjects();

  // Set up display size (to accommodate for window resizing)
  ImGuiIO &io = ImGui::GetIO();
  int width, height, displayWidth, displayHeight;
  SDL_GetWindowSize(window, &width, &height);
  SDL_GL_GetDrawableSize(window, &displayWidth, &displayHeight);
  io.DisplaySize = ImVec2((float)width, (float)height);
  io.DisplayFramebufferScale = ImVec2(width > 0 ? ((float)displayWidth / width) : 0,
    height > 0 ? ((float)displayHeight / height) : 0);

  // Set up time step
  Uint32 time = SDL_GetTicks();
  double currentTime = time / 1000.0;
  io.DeltaTime = time_ > 0.0 ? static_cast<float>(currentTime - time_) : (float)(1.0f / 60.0f);
  time_ = currentTime;

  // Set up inputs
  int mouseX, mouseY;
  Uint32 mouseMask = SDL_GetMouseState(&mouseX, &mouseY);
  if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
    io.MousePos = ImVec2((float)mouseX, (float)mouseY);
  else
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

  // If a mouse press event came, always pass it
  // as "mouse held this frame", so we don't miss
  // click-release events that are shorter than 1
  // frame.
  io.MouseDown[0] = mousePressed_[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  io.MouseDown[1] = mousePressed_[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  io.MouseDown[2] = mousePressed_[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  mousePressed_[0] = mousePressed_[1] = mousePressed_[2] = false;
  io.MouseWheel = mouseWheel_;
  mouseWheel_ = 0.0f;

  // Hide OS mouse cursor if ImGui is drawing it
  SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

  // This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag
  // that you can use to dispatch inputs to your application.
  ImGui::NewFrame();
}
