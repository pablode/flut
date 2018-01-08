set(IMGUI_SRCS
  imconfig.h
  imgui.cpp
  imgui.h
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_impl_sdl_glew.cpp
  imgui_impl_sdl_glew.h
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
  ${GLEW_LIBRARY}
)

target_include_directories(
  imgui PUBLIC
  ${SDL2_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
)
