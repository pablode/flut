cmake_minimum_required(VERSION 3.11)

project(3rdparty)

set(3RDPARTY_INSTALL_DIR "${PROJECT_SOURCE_DIR}/INSTALL")
set(3RDPARTY_BINARY_DIR "${CMAKE_BINARY_DIR}/bin")
set(3RDPARTY_SOURCE_DIR "${PROJECT_SOURCE_DIR}")

add_subdirectory(scripts)
