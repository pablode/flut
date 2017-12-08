#include "core/Window.hpp"
#include "Renderer.hpp"

#include <iostream>
#include <fstream>
#include <vector>

int main(int argc, char *argv[]) {
  ansimproj::core::Window window{"TestWindow", 800, 600};
  ansimproj::Renderer renderer{};

  while (!window.shouldClose()) {
    window.pollEvents();
    renderer.render();
    window.swap();
  }
  return EXIT_SUCCESS;
}
