set(ANSIMPROJ_SRCS
  main.cpp
  core/BaseRenderer.cpp
  core/BaseRenderer.hpp
  core/Camera.cpp
  core/Camera.hpp
  core/Utils.hpp
  core/Window.cpp
  core/Window.hpp
  Simulation.cpp
  Simulation.hpp
)

add_subdirectory(imgui)

add_executable(ansimproj WIN32 ${ANSIMPROJ_SRCS})

if (CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(ansimproj PRIVATE -Wall)
  target_compile_options(ansimproj PRIVATE -Wextra)
  target_compile_options(ansimproj PRIVATE -Werror)
  target_compile_options(ansimproj PRIVATE -Wno-unused-parameter)
  target_compile_options(ansimproj PRIVATE -Wno-reorder)
  target_compile_options(ansimproj PRIVATE -msse2)
elseif(MSVC)
  target_compile_options(ansimproj PRIVATE /Wall)
  target_compile_options(ansimproj PRIVATE /arch:SSE2)
endif()

target_compile_definitions(
  ansimproj PRIVATE
  RESOURCES_PATH="${RESOURCES_PATH}"
)

target_link_libraries(
  ansimproj
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
  ${GLBINDING_LIBRARIES}
  imgui
  SDL2
  SDL2main
  glbinding::glbinding
)

target_include_directories(
  ansimproj PUBLIC
  imgui
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${EIGEN_INCLUDE_DIR}
)
