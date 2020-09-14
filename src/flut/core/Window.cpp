#include "Window.hpp"

#include <imgui.h>
#include <imgui_impl_sdl_glad.h>
#include <glad/glad.h>
#include <stdexcept>
#include <cstdio>

using namespace flut;
using namespace flut::core;

Window::Window(const char* title, std::uint32_t width, std::uint32_t height)
  : shouldClose_{false}
{
  if (SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    throw std::runtime_error(SDL_GetError());
  }

  window_ = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    width,
    height,
    SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
  );
  if (!window_) {
    throw std::runtime_error(SDL_GetError());
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef NDEBUG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_NO_ERROR, 1);
#else
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
  context_ = SDL_GL_CreateContext(window_);
  if (!context_) {
    throw std::runtime_error(SDL_GetError());
  }

  if (SDL_GL_SetSwapInterval(1)) {
    std::printf("Warning: Unable to activate VSync.\n");
  }

  if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
    throw std::runtime_error("Unable to initialize Glad.");
  }

  if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 6)) {
    throw std::runtime_error("OpenGL 4.6 required.");
  }

  std::printf("OpenGL Version %d.%d loaded.\n", GLVersion.major, GLVersion.minor);

  ImGui_ImplSdlGlad_Init(window_);
  ImGui_ImplSdlGlad_NewFrame(window_);
}

Window::~Window()
{
  ImGui_ImplSdlGlad_Shutdown();
  SDL_GL_DeleteContext(context_);
  SDL_DestroyWindow(window_);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Window::pollEvents()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    if (ImGui_ImplSdlGlad_ProcessEvent(&event)) {
      continue;
    }

    if (event.type == SDL_QUIT) {
      shouldClose_ = true;
      continue;
    }
    if (event.type != SDL_WINDOWEVENT) {
      continue;
    }

    switch (event.window.event)
    {
    case SDL_WINDOWEVENT_RESIZED:
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      const auto width = static_cast<std::uint32_t>(event.window.data1);
      const auto height = static_cast<std::uint32_t>(event.window.data2);
      if (resizeCallback_) {
        resizeCallback_(width, height);
      }
      break;
    }
  }
}

bool Window::shouldClose()
{
  return shouldClose_;
}

void Window::swap()
{
  ImGui::Render();
  SDL_GL_SwapWindow(window_);
  ImGui_ImplSdlGlad_NewFrame(window_);
}

std::uint32_t Window::width() const
{
  int width;
  SDL_GL_GetDrawableSize(window_, &width, nullptr);
  return static_cast<std::uint32_t>(width);
}

std::uint32_t Window::height() const
{
  int height;
  SDL_GL_GetDrawableSize(window_, nullptr, &height);
  return static_cast<std::uint32_t>(height);
}

std::int32_t Window::mouseX() const
{
  int x;
  SDL_GetMouseState(&x, nullptr);
  return x;
}

std::int32_t Window::mouseY() const
{
  int y;
  SDL_GetMouseState(nullptr, &y);
  return y;
}

bool Window::mouseDown() const
{
  return (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
}

bool Window::keyUp() const
{
  const auto& states = SDL_GetKeyboardState(nullptr);
  return states[SDL_SCANCODE_W] != 0;
}

bool Window::keyDown() const
{
  const auto& states = SDL_GetKeyboardState(nullptr);
  return states[SDL_SCANCODE_S] != 0;
}

void Window::resize(std::function<void(std::uint32_t, std::uint32_t)> callback)
{
  resizeCallback_ = callback;
}
