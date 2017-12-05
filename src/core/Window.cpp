#include "Window.hpp"

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

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
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

std::int32_t ansimproj::core::Window::width() {
  int width;
  SDL_GL_GetDrawableSize(window_, &width, nullptr);
  return width;
}

std::int32_t ansimproj::core::Window::height() {
  int height;
  SDL_GL_GetDrawableSize(window_, nullptr, &height);
  return height;
}
