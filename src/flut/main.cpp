#include "Simulation.hpp"
#include "Camera.hpp"
#include "Window.hpp"
#include "GlQueryRetriever.hpp"

#include <imgui.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <vector>

using namespace flut;

int main(int argc, char* argv[])
{
  constexpr std::uint32_t WIDTH = 1200;
  constexpr std::uint32_t HEIGHT = 800;

  Window window{"flut", WIDTH, HEIGHT};
  Camera camera{window};
  Simulation simulation{WIDTH, HEIGHT};

  window.resize([&](std::uint32_t width, std::uint32_t height) {
    simulation.resize(width, height);
  });

  auto& options = simulation.options();
  auto& times = simulation.times();
  using clock = std::chrono::high_resolution_clock;
  auto lastTime = clock::now();

  int ipF = 8;

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

    ImGui::Text("Particles: %d", simulation.particleCount());
    ImGui::Text("Delta-time: %f", simulation.DT * options.deltaTimeMod);
    ImGui::Text("Grid: %dx%dx%d", simulation.GRID_RES.x, simulation.GRID_RES.y, simulation.GRID_RES.z);
    ImGui::Text("Frame: %.2fms", deltaTime * 1000.0f);

    ImGui::Text("Step 1  Step 2  Step 3  Step 4  Step 5  Step 6  Render");
    ImGui::Text("%.2fms  %.2fms  %.2fms  %.2fms  %.2fms  %.2fms  %.2fms",
                times.simStempMs[0], times.simStempMs[1], times.simStempMs[2],
                times.simStempMs[3], times.simStempMs[4], times.simStempMs[5], times.renderMs);

    ImGui::SliderFloat("Delta-Time mod", &options.deltaTimeMod, 0.0f, 2.0f, nullptr, 1.0f);

    ImGui::DragInt("Integrations per Frame", &ipF, 1.0f, 0, GlQueryRetriever::MAX_SIM_ITERS_PER_FRAME);

    ImGui::DragFloat3("Gravity", &options.gravity[0], 0.075f, -10.0f, 10.0f, nullptr, 1.0f);

    ImGui::Text("Particle Color:");
    ImGui::RadioButton("Initial", &options.colorMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Velocity", &options.colorMode, 1);
    ImGui::SameLine();
    ImGui::RadioButton("Speed", &options.colorMode, 2);
    ImGui::RadioButton("Density", &options.colorMode, 3);
    ImGui::SameLine();
    ImGui::RadioButton("Uniform Grid", &options.colorMode, 4);

    ImGui::DragFloat("Point scale", &options.pointScale, 0.01f, 0.1f, 2.5f);

    ImGui::End();

    window.swap();
  }

  return EXIT_SUCCESS;
}
