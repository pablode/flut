#include "core/Window.hpp"
#include "Renderer.hpp"

#include <iostream>
#include <fstream>
#include <vector>

std::vector<char> loadFileText(const std::string &filePath) {
  std::ifstream file(filePath, std::ios_base::in | std::ios_base::binary);
  if (!file.is_open())
    return {};
  static std::vector<char> result;
  file.seekg(0, std::ios_base::end);
  result.resize(file.tellg());
  file.seekg(0, std::ios_base::beg);
  file.read(result.data(), result.size());
  return result;
}

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
