cmake_minimum_required(VERSION 3.11)

project(ansimproj VERSION 0.0.1)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

set(THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty")
set(THIRDPARTY_INSTALL_DIR "${THIRDPARTY_DIR}/INSTALL")
set(CMAKE_PREFIX_PATH "${THIRDPARTY_INSTALL_DIR}")

set(RESOURCES_PATH ${CMAKE_SOURCE_DIR}/res
  CACHE PATH "Location of the resources folder.")

include(CheckIPOSupported)
check_ipo_supported(RESULT ENABLE_IPO)
if (ENABLE_IPO)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()

find_package(OpenGL REQUIRED)
find_package(Eigen REQUIRED)
find_package(SDL2 REQUIRED)
find_package(glbinding REQUIRED)

add_subdirectory(src)
