#include "Window.hpp"

#include <stdexcept>

ansimproj::core::Window::Window(std::string title, std::int32_t width, std::int32_t height)
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
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifndef NDEBUG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
  context_ = SDL_GL_CreateContext(window_);
  if (!context_) {
    throw std::runtime_error(SDL_GetError());
  }
}

ansimproj::core::Window::~Window() {
  SDL_GL_DeleteContext(context_);
  SDL_DestroyWindow(window_);
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void ansimproj::core::Window::pollEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      shouldClose_ = true;
      break;
    case SDL_WINDOWEVENT: {
      switch (event.window.event) {
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED: {
        const std::uint32_t width = static_cast<std::uint32_t>(event.window.data1);
        const std::uint32_t height = static_cast<std::uint32_t>(event.window.data2);
        if (resizeCallback_) {
          resizeCallback_(width, height);
        }
      } break;
      default:
        break;
      }
      break;
    }
    case SDL_MOUSEBUTTONDOWN: {
      // TODO
      break;
    }
    default:
      break;
    }
  }
}

bool ansimproj::core::Window::shouldClose() {
  return shouldClose_;
}

void ansimproj::core::Window::swap() {
  SDL_GL_SwapWindow(window_);
}

std::uint32_t ansimproj::core::Window::width() const {
  int width;
  SDL_GL_GetDrawableSize(window_, &width, nullptr);
  return static_cast<std::uint32_t>(width);
}

std::uint32_t ansimproj::core::Window::height() const {
  int height;
  SDL_GL_GetDrawableSize(window_, nullptr, &height);
  return static_cast<std::uint32_t>(height);
}

std::int32_t ansimproj::core::Window::mouseX() const {
  int x;
  SDL_GetMouseState(&x, nullptr);
  return x;
}

std::int32_t ansimproj::core::Window::mouseY() const {
  int y;
  SDL_GetMouseState(nullptr, &y);
  return y;
}

bool ansimproj::core::Window::mouseDown() const {
  return static_cast<bool>(SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT));
}

void ansimproj::core::Window::resize(std::function<void(std::uint32_t, std::uint32_t)> callback) {
  resizeCallback_ = callback;
}
