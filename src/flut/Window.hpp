#pragma once

#include <SDL.h>
#include <functional>
#include <string>

namespace flut
{
  class Window
  {
  public:
    Window(const char* title, std::uint32_t width, std::uint32_t height);

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
    bool m_shouldClose;
    SDL_Window* m_window;
    SDL_GLContext m_context;
    std::function<void(std::uint32_t, std::uint32_t)> m_resizeCallback;
  };
}
