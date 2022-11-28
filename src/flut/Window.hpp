#pragma once

#include <SDL.h>
#include <functional>
#include <string>

namespace flut
{
  class Window
  {
  public:
    Window(const char* title, uint32_t width, uint32_t height);

    ~Window();

  public:
    void pollEvents();

    bool shouldClose();

    void swap();

    uint32_t width() const;

    uint32_t height() const;

    int32_t mouseX() const;

    int32_t mouseY() const;

    bool mouseDown() const;

    bool keyUp() const;

    bool keyDown() const;

    void resize(std::function<void(uint32_t, uint32_t)> callback);

  private:
    bool m_shouldClose;
    SDL_Window* m_window;
    SDL_GLContext m_context;
    std::function<void(uint32_t, uint32_t)> m_resizeCallback;
  };
}
