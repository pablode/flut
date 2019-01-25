set(ANSIMPROJ_SRCS
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

add_executable(ansimproj WIN32 ${ANSIMPROJ_SRCS})

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(ansimproj PRIVATE -Wall)
  target_compile_options(ansimproj PRIVATE -Wextra)
  target_compile_options(ansimproj PRIVATE -Werror)
  target_compile_options(ansimproj PRIVATE -Wno-unused-parameter)
  target_compile_options(ansimproj PRIVATE -Wno-reorder)
  target_compile_options(ansimproj PRIVATE -Wno-error=int-in-bool-context)
elseif(MSVC)
  target_compile_options(ansimproj PRIVATE /MP)
  target_compile_options(ansimproj PRIVATE /Wall)
  target_compile_options(ansimproj PRIVATE /D_USE_MATH_DEFINES)
  target_compile_options(ansimproj PRIVATE /DNOMINMAX)
endif()

target_compile_definitions(
  ansimproj PRIVATE
  RESOURCES_PATH="${RESOURCES_PATH}"
)

target_link_libraries(
  ansimproj PRIVATE
  imgui
  SDL2::SDL2
  SDL2::SDL2main
  Eigen::Eigen
  glbinding::glbinding
  OpenGL::GL
)

target_include_directories(
  ansimproj PRIVATE
  ${PROJECT_SOURCE_DIR}/src
)
