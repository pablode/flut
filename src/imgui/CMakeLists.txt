set(IMGUI_SRCS
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

add_library(imgui STATIC ${IMGUI_SRCS})

target_link_libraries(
  imgui PUBLIC
  ${SDL2_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${glbinding_LIBRARIES}
)

target_include_directories(
  imgui PUBLIC
  ${SDL2_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${glbinding_INCLUDE_DIR}
)
