set(GLAD_INCLUDES
  include/glad/glad.h
  include/KHR/khrplatform.h
)

set(GLAD_SRCS
  src/glad.c
)

add_library(
  glad
  STATIC
  ${GLAD_INCLUDES}
  ${GLAD_SRCS}
)

target_include_directories(
  glad
  PUBLIC
  include
  PRIVATE
  src
)
