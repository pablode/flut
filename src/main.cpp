#include "core/Window.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
  ansimproj::core::Window window{"TestWindow", 800, 600};
  while (!window.shouldClose()) {
    window.pollEvents();
  }
  return EXIT_SUCCESS;
}
