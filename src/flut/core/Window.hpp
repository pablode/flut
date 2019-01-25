#pragma once

#include <SDL.h>
#include <functional>
#include <string>

namespace flut {
  namespace core {

    class Window {

    public:
      Window(std::string title, std::uint32_t width, std::uint32_t height);

      ~Window();

    public:
      void pollEvents();

      bool shouldClose();

      void swap();

      std::uint32_t width() const;

      std::uint32_t height() const;

      std::int32_t mouseX() const;

      std::int32_t mouseY() const;

      bool mouseDown() const;

      bool keyUp() const;

      bool keyDown() const;

      void resize(std::function<void(std::uint32_t, std::uint32_t)> callback);

    private:
      bool shouldClose_;
      SDL_Window *window_;
      SDL_GLContext context_;
      std::function<void(std::uint32_t, std::uint32_t)> resizeCallback_;
    };
  }
}
