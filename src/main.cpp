#include "Simulation.hpp"
#include "core/Camera.hpp"
#include "core/Window.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  ansimproj::core::Window window{"ansimproj", 800, 600};
  ansimproj::core::Camera camera{window};
  ansimproj::Simulation simulation{};

  window.resize([&](std::uint32_t width, std::uint32_t height) { simulation.resize(width, height); });
  simulation.resize(window.width(), window.height());

  using clock = std::chrono::high_resolution_clock;
  auto lastTime = clock::now();
  while (!window.shouldClose()) {
    window.pollEvents();

    std::chrono::duration<double> timeSpan(clock::now() - lastTime);
    double deltaTime = timeSpan.count();

    simulation.render(camera);
    camera.update(deltaTime);
    window.swap();
  }
  return EXIT_SUCCESS;
}
