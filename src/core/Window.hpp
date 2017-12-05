#pragma once

#include <SDL/SDL.h>
#include <string>

namespace ansimproj {
  namespace core {

    class Window {

    public:
      Window(std::string title, std::int32_t width, std::int32_t height);

      ~Window();

    public:
      void pollEvents();

      bool shouldClose();

      void swap();

      std::int32_t width();

      std::int32_t height();

    private:
      bool shouldClose_;
      SDL_Window *window_;
      SDL_GLContext context_;
    };
  }
}
