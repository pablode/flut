#include "Simulation.hpp"
#include "Camera.hpp"
#include "Window.hpp"

#include <imgui.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
  constexpr std::uint32_t WIDTH = 1200;
  constexpr std::uint32_t HEIGHT = 800;

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

  int ipF = 5;

  while (!window.shouldClose())
  {
    const std::chrono::duration<float> timeSpan{clock::now() - lastTime};
    const float deltaTime = timeSpan.count();
    lastTime = clock::now();

    // IO
    window.pollEvents();
    const auto& imguiIO = ImGui::GetIO();

    if (!imguiIO.WantCaptureMouse && !imguiIO.WantCaptureKeyboard)
    {
      camera.update(deltaTime);
    }

    // Simulation / Render
    simulation.setIntegrationsPerFrame(ipF);

    simulation.render(camera, deltaTime);

    // UI
    ImGui::SetNextWindowPos({50, 50});
    ImGui::Begin("SPH GPU Fluid Simulation", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    const float frameTime = times.simStep1Ms + times.simStep2Ms + times.simStep3Ms + times.simStep4Ms +
                            times.simStep5Ms + times.simStep6Ms + times.renderMs;
    ImGui::Text("Particles: %d", simulation.PARTICLE_COUNT);
    ImGui::Text("Delta-time: %f", simulation.DT * options.deltaTimeMod);
    ImGui::Text("Grid: %dx%dx%d", simulation.GRID_RES.x, simulation.GRID_RES.y, simulation.GRID_RES.z);
    ImGui::Text("Frame: %.2fms (%.2fms)", frameTime, deltaTime * 1000.0f);

    ImGui::Text("Step 1  Step 2  Step 3  Step 4  Step 5  Step 6  Render");
    ImGui::Text("%.2fms  %.2fms  %.2fms  %.2fms  %.2fms  %.2fms  %.2fms",
                times.simStep1Ms, times.simStep2Ms, times.simStep3Ms,
                times.simStep4Ms, times.simStep5Ms, times.simStep6Ms, times.renderMs);

    ImGui::SliderFloat("Delta-Time mod", &options.deltaTimeMod, 0.0f, 2.0f, nullptr, 1.0f);

    ImGui::DragInt("Integrations per Frame", &ipF, 1.0f, 0, 20);

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
