add_library(
  imgui STATIC
  include/imgui.h
  include/imconfig.h
  include/imgui_impl_sdl_glad.h
  src/imgui.cpp
  src/imgui_demo.cpp
  src/imgui_draw.cpp
  src/imgui_impl_sdl_glad.cpp
  src/imgui_internal.h
  src/stb_rect_pack.h
  src/stb_textedit.h
  src/stb_truetype.h
)

target_link_libraries(
  imgui PRIVATE
  SDL2
  OpenGL::GL
  glad
)

target_include_directories(
  imgui
  PUBLIC
  include
  PRIVATE
  src
)
