set(ANSIMPROJ_SRCS
  main.cpp
  core/Camera.cpp
  core/Camera.hpp
  core/Image.cpp
  core/Image.hpp
  core/Window.cpp
  core/Window.hpp
  Renderer.cpp
  Renderer.hpp
)

add_executable(ansimproj ${ANSIMPROJ_SRCS})

target_compile_definitions(
  ansimproj PRIVATE
  RESOURCES_PATH="${RESOURCES_PATH}"
)

target_include_directories(
  ansimproj PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  ansimproj
)
