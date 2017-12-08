#include "Renderer.hpp"
#include "core/Camera.hpp"
#include "core/Window.hpp"

#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  ansimproj::core::Window window{"ansimproj", 800, 600};
  ansimproj::core::Camera camera{window};
  ansimproj::Renderer renderer{};

  window.resize([&](std::uint32_t width, std::uint32_t height) { renderer.resize(width, height); });

  renderer.resize(window.width(), window.height());
  while (!window.shouldClose()) {
    window.pollEvents();
    renderer.render(camera);
    camera.update(0.01f);
    window.swap();
  }
  return EXIT_SUCCESS;
}
