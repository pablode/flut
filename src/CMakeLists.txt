set(ANSIMPROJ_SRCS
  main.cpp
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
