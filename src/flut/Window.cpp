#include "Window.hpp"

#include <imgui.h>
#include <imgui_impl_sdl_glad.h>
#include <glad/glad.h>
#include <stdexcept>
#include <cstdio>

using namespace flut;

Window::Window(const char* title, std::uint32_t width, std::uint32_t height)
  : m_shouldClose{false}
{
  if (SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    throw std::runtime_error(SDL_GetError());
  }

  m_window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    width,
    height,
    SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL
  );

  if (!m_window) {
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

  m_context = SDL_GL_CreateContext(m_window);

  if (!m_context) {
    throw std::runtime_error(SDL_GetError());
  }

  if (SDL_GL_SetSwapInterval(1)) {
    printf("Warning: Unable to activate VSync.\n");
  }

  if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
    throw std::runtime_error("Unable to initialize Glad.");
  }

  if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 6)) {
    throw std::runtime_error("OpenGL 4.6 required.");
  }

  if (!GLAD_GL_ARB_bindless_texture) {
    throw std::runtime_error("GL_ARB_bindless_texture extension is required.");
  }

  printf("OpenGL Version %d.%d loaded.\n", GLVersion.major, GLVersion.minor);
  fflush(stdout);

  ImGui_ImplSdlGlad_Init(m_window);
  ImGui_ImplSdlGlad_NewFrame(m_window);
}

Window::~Window()
{
  ImGui_ImplSdlGlad_Shutdown();
  SDL_GL_DeleteContext(m_context);
  SDL_DestroyWindow(m_window);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void Window::pollEvents()
{
  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    if (ImGui_ImplSdlGlad_ProcessEvent(&event))
    {
      continue;
    }

    if (event.type == SDL_QUIT)
    {
      m_shouldClose = true;
      continue;
    }
    if (event.type != SDL_WINDOWEVENT)
    {
      continue;
    }

    switch (event.window.event)
    {
    case SDL_WINDOWEVENT_RESIZED:
    case SDL_WINDOWEVENT_SIZE_CHANGED:
      const auto width = static_cast<std::uint32_t>(event.window.data1);
      const auto height = static_cast<std::uint32_t>(event.window.data2);
      if (m_resizeCallback) {
        m_resizeCallback(width, height);
      }
      break;
    }
  }
}

bool Window::shouldClose()
{
  return m_shouldClose;
}

void Window::swap()
{
  ImGui::Render();
  SDL_GL_SwapWindow(m_window);
  ImGui_ImplSdlGlad_NewFrame(m_window);
}

std::uint32_t Window::width() const
{
  int width;
  SDL_GL_GetDrawableSize(m_window, &width, nullptr);
  return static_cast<std::uint32_t>(width);
}

std::uint32_t Window::height() const
{
  int height;
  SDL_GL_GetDrawableSize(m_window, nullptr, &height);
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
  m_resizeCallback = callback;
}
