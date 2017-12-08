#include "BaseRenderer.hpp"
#include "core/Window.hpp"

#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  ansimproj::core::Window window{"TestWindow", 800, 600};
  ansimproj::BaseRenderer renderer{};

  while (!window.shouldClose()) {
    window.pollEvents();
    renderer.render();
    window.swap();
  }
  return EXIT_SUCCESS;
}
