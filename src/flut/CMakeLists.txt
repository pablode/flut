set(FLUT_SRCS
  main.cpp
  core/Camera.cpp
  core/Camera.hpp
  core/SimulationBase.cpp
  core/SimulationBase.hpp
  core/Utils.hpp
  core/Window.cpp
  core/Window.hpp
  Simulation.cpp
  Simulation.hpp
)

add_executable(flut WIN32 ${FLUT_SRCS})

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(flut PRIVATE -Wall)
  target_compile_options(flut PRIVATE -Wextra)
  target_compile_options(flut PRIVATE -Werror)
  target_compile_options(flut PRIVATE -Wno-unused-parameter)
  target_compile_options(flut PRIVATE -Wno-reorder)
  target_compile_options(flut PRIVATE -Wno-error=int-in-bool-context)
elseif(MSVC)
  target_compile_options(flut PRIVATE /MP)
  target_compile_options(flut PRIVATE /Wall)
  target_compile_options(flut PRIVATE /D_USE_MATH_DEFINES)
  target_compile_options(flut PRIVATE /DNOMINMAX)
endif()

target_compile_definitions(
  flut PRIVATE
  RESOURCES_PATH="${RESOURCES_PATH}"
)

target_link_libraries(
  flut PRIVATE
  imgui
  SDL2::SDL2
  SDL2::SDL2main
  Eigen::Eigen
  glbinding::glbinding
  OpenGL::GL
)

target_include_directories(
  flut PRIVATE
  ${PROJECT_SOURCE_DIR}/src
)
