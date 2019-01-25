#include "Window.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl_glbinding.h>
#include <glbinding/Binding.h>
#include <stdexcept>
#include <cstdio>

flut::core::Window::Window(std::string title, std::uint32_t width, std::uint32_t height)
  : shouldClose_{false} {
  if (SDL_InitSubSystem(SDL_INIT_VIDEO)) {
    throw std::runtime_error(SDL_GetError());
  }

  window_ = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
    height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
  const auto vsyncSucc = SDL_GL_SetSwapInterval(1);
  if (vsyncSucc != 0) {
    std::printf("Warning: Unable to activate VSync.\n");
  }

  glbinding::Binding::initialize();
  ImGui_ImplSdlGlBinding_Init(window_);
  ImGui_ImplSdlGlBinding_NewFrame(window_);
}

flut::core::Window::~Window() {
  ImGui_ImplSdlGlBinding_Shutdown();
  SDL_GL_DeleteContext(context_);
  SDL_DestroyWindow(window_);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void flut::core::Window::pollEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (!ImGui_ImplSdlGlBinding_ProcessEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        shouldClose_ = true;
        break;
      case SDL_WINDOWEVENT: {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED:
        case SDL_WINDOWEVENT_SIZE_CHANGED: {
          const auto width = static_cast<std::uint32_t>(event.window.data1);
          const auto height = static_cast<std::uint32_t>(event.window.data2);
          if (resizeCallback_) {
            resizeCallback_(width, height);
          }
        } break;
        default:
          break;
        }
        break;
      }
      default:
        break;
      }
    }
  }
}

bool flut::core::Window::shouldClose() {
  return shouldClose_;
}

void flut::core::Window::swap() {
  ImGui::Render();
  SDL_GL_SwapWindow(window_);
  ImGui_ImplSdlGlBinding_NewFrame(window_);
}

std::uint32_t flut::core::Window::width() const {
  int width;
  SDL_GL_GetDrawableSize(window_, &width, nullptr);
  return static_cast<std::uint32_t>(width);
}

std::uint32_t flut::core::Window::height() const {
  int height;
  SDL_GL_GetDrawableSize(window_, nullptr, &height);
  return static_cast<std::uint32_t>(height);
}

std::int32_t flut::core::Window::mouseX() const {
  int x;
  SDL_GetMouseState(&x, nullptr);
  return x;
}

std::int32_t flut::core::Window::mouseY() const {
  int y;
  SDL_GetMouseState(nullptr, &y);
  return y;
}

bool flut::core::Window::mouseDown() const {
  return static_cast<bool>(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT));
}

bool flut::core::Window::keyUp() const {
  const auto& states = SDL_GetKeyboardState(nullptr);
  return static_cast<bool>(states[SDL_SCANCODE_W]);
}

bool flut::core::Window::keyDown() const {
  const auto& states = SDL_GetKeyboardState(nullptr);
  return static_cast<bool>(states[SDL_SCANCODE_S]);
}

void flut::core::Window::resize(std::function<void(std::uint32_t, std::uint32_t)> callback) {
  resizeCallback_ = callback;
}
