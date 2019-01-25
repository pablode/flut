cmake_minimum_required(VERSION 3.11)

project(3rdparty)

set(3RDPARTY_INSTALL_DIR "${PROJECT_SOURCE_DIR}/INSTALL"
  CACHE STRING "Output location for headers, libraries and executables.")
set(3RDPARTY_SOURCE_DIR "${CMAKE_BINARY_DIR}/src")
set(3RDPARTY_ROOT "${PROJECT_SOURCE_DIR}")

add_subdirectory(scripts)
