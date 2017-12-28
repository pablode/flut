#include "Simulation.hpp"
#include "core/Camera.hpp"
#include "core/Window.hpp"

#include "imgui/imgui.h"

#include <chrono>
#include <fstream>
#include <glbinding/Binding.h>
#include <iostream>
#include <vector>

int main(int argc, char *argv[]) {
  glbinding::Binding::initialize();
  ansimproj::core::Window window{"ansimproj", 1200, 800};
  ansimproj::core::Camera camera{window};
  ansimproj::Simulation simulation{};

  window.resize(
    [&](std::uint32_t width, std::uint32_t height) { simulation.resize(width, height); });
  simulation.resize(window.width(), window.height());

  auto &options = simulation.options();
  using clock = std::chrono::high_resolution_clock;
  auto lastTime = clock::now();
  while (!window.shouldClose()) {
    std::chrono::duration<float> timeSpan(clock::now() - lastTime);
    float deltaTime = timeSpan.count();
    lastTime = clock::now();

    // IO
    window.pollEvents();
    const auto &imguiIO = ImGui::GetIO();
    if (!imguiIO.WantCaptureMouse && !imguiIO.WantCaptureKeyboard) {
      camera.update(deltaTime);
    }

    // Simulation / Render
    simulation.render(camera, deltaTime);

    // UI
    ImGui::SetNextWindowPos({50, 50});
    ImGui::Begin("SPH GPU Fluid Simulation", nullptr,
      ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    ImGui::SliderFloat("Delta-Time mod", &options.deltaTimeMod, 0.0f, 5.0f, nullptr, 2.0f);

    ImGui::DragFloat3("Gravity", &options.gravity[0], 0.1f);

    ImGui::Text("Particle Color:");
    ImGui::RadioButton("Initial", &options.colorMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Density", &options.colorMode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Uniform Grid", &options.colorMode, 2);

    ImGui::Text("Particle Shading:");
    ImGui::RadioButton("None", &options.shadingMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Phong", &options.shadingMode, 1);

    ImGui::End();

    // TODO: get property struct from simulation, modify, copy back

    window.swap();
  }

  return EXIT_SUCCESS;
}
