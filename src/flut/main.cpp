#include "Simulation.hpp"
#include "core/Camera.hpp"
#include "core/Window.hpp"

#include <imgui.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
  constexpr std::uint32_t WIDTH = 800;
  constexpr std::uint32_t HEIGHT = 600;

  flut::core::Window window{"flut", WIDTH, HEIGHT};
  flut::core::Camera camera{window};
  flut::Simulation simulation{WIDTH, HEIGHT};

  window.resize([&](std::uint32_t width, std::uint32_t height) {
    simulation.resize(width, height);
  });

  bool wasPreset1ButtonPressed = false;
  auto& options = simulation.options();
  auto& times = simulation.times();
  using clock = std::chrono::high_resolution_clock;
  auto lastTime = clock::now();

  while (!window.shouldClose())
  {
    const std::chrono::duration<float> timeSpan{clock::now() - lastTime};
    const float deltaTime = timeSpan.count();
    lastTime = clock::now();

    // IO
    window.pollEvents();
    const auto& imguiIO = ImGui::GetIO();
    if (!imguiIO.WantCaptureMouse && !imguiIO.WantCaptureKeyboard) {
      camera.update(deltaTime);
    }

    // Simulation / Render
    simulation.render(camera, deltaTime);

    // UI
    ImGui::SetNextWindowPos({50, 50});
    ImGui::Begin("SPH GPU Fluid Simulation", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    const float frameTime = times.gridInsertMs + times.gridSortMs + times.gridIndexingMs + times.densityComputationMs +
      times.forceUpdateMs + times.renderingMs;
    ImGui::Text("Particles: %d", simulation.PARTICLE_COUNT);
    ImGui::Text("Grid: %dx%dx%d", simulation.GRID_RES(0), simulation.GRID_RES(1), simulation.GRID_RES(2));
    ImGui::Text("Frame: %.2fms (%.2fms)", frameTime, deltaTime * 1000.0f);

    ImGui::Text("GrIns   GrSort  GrIdx   DensCom  ForceUp  Render");
    ImGui::Text("%.2fms  %.2fms  %.2fms  %.2fms   %.2fms   %.2fms", times.gridInsertMs, times.gridSortMs,
      times.gridIndexingMs, times.densityComputationMs, times.forceUpdateMs, times.renderingMs);

    ImGui::SliderFloat("Delta-Time mod", &options.deltaTimeMod, 0.0f, 5.0f, nullptr, 2.0f);

    ImGui::DragFloat3("Gravity", &options.gravity[0], 0.075f, -10.0f, 10.0f, nullptr, 1.0f);

    const bool preset1ButtonPressed = ImGui::Button("Preset 1");
    if (!wasPreset1ButtonPressed && preset1ButtonPressed) {
      simulation.preset1();
    }
    wasPreset1ButtonPressed = preset1ButtonPressed;

    ImGui::Text("Particle Color:");
    ImGui::RadioButton("Initial", &options.colorMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Velocity", &options.colorMode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Speed", &options.colorMode, 2);
    ImGui::RadioButton("Density", &options.colorMode, 3);
    ImGui::SameLine();
    ImGui::RadioButton("Uniform Grid", &options.colorMode, 4);

    ImGui::Text("Particle Display:");
    ImGui::RadioButton("Flat", &options.shadingMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Fluid", &options.shadingMode, 1);

    ImGui::End();

    window.swap();
  }

  return EXIT_SUCCESS;
}
