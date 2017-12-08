set(ANSIMPROJ_SRCS
  main.cpp
  BaseRenderer.cpp
  BaseRenderer.hpp
  core/Camera.cpp
  core/Camera.hpp
  core/Image.cpp
  core/Image.hpp
  core/Utils.hpp
  core/Window.cpp
  core/Window.hpp
  Renderer.cpp
  Renderer.hpp
)

add_executable(ansimproj WIN32 ${ANSIMPROJ_SRCS})

target_compile_definitions(
  ansimproj PRIVATE
  RESOURCES_PATH="${RESOURCES_PATH}"
)

target_link_libraries(
  ansimproj
  ${SDL2_LIBRARY}
  ${SDL2MAIN_LIBRARY}
  ${GLBINDING_LIBRARIES}
)

target_include_directories(
  ansimproj PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${EIGEN_INCLUDE_DIR}
)

