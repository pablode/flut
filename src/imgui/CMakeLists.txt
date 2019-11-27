add_library(
  imgui STATIC
  imconfig.h
  imgui.cpp
  imgui.h
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_impl_sdl_glbinding.cpp
  imgui_impl_sdl_glbinding.h
  imgui_internal.h
  stb_rect_pack.h
  stb_textedit.h
  stb_truetype.h
)

target_link_libraries(
  imgui PUBLIC
  SDL2
  OpenGL::GL
  glbinding
)

target_include_directories(
  imgui PRIVATE
  ${FLUT_SOURCE_DIR}
)
