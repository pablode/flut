#include "core/Window.hpp"
#include "Renderer.hpp"

#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  ansimproj::core::Window window{"TestWindow", 800, 600};
  ansimproj::Renderer renderer{};

  window.resize([&](std::uint32_t width, std::uint32_t height) {
    renderer.resize(width, height);
  });
  renderer.resize(window.width(), window.height());
  while (!window.shouldClose()) {
    window.pollEvents();
    renderer.render();
    window.swap();
  }
  return EXIT_SUCCESS;
}
