add_executable(
  flut WIN32
  main.cpp
  core/Camera.cpp
  core/Camera.hpp
  core/SimulationBase.cpp
  core/SimulationBase.hpp
  core/Window.cpp
  core/Window.hpp
  Simulation.cpp
  Simulation.hpp
)

if(MSVC)
  target_compile_options(flut PRIVATE /MP)
  target_compile_options(flut PRIVATE /Wall)
  target_compile_options(flut PRIVATE /D_USE_MATH_DEFINES)
  target_compile_options(flut PRIVATE /DNOMINMAX)
else()
  target_compile_options(flut PRIVATE -Wall)
  target_compile_options(flut PRIVATE -Wextra)
  target_compile_options(flut PRIVATE -Wno-unused-parameter)
  target_compile_options(flut PRIVATE -Wno-reorder)
  target_compile_options(flut PRIVATE -Wno-error=int-in-bool-context)
endif()

target_compile_definitions(
  flut PRIVATE
  RESOURCES_DIR="${FLUT_RESOURCES_DIR}"
)

target_link_libraries(
  flut PRIVATE
  imgui
  SDL2
  SDL2main
  glm
  glad
  OpenGL::GL
)
