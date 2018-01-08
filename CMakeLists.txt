cmake_minimum_required(VERSION 3.5)

project(ansimproj VERSION 0.0.1)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

set(RESOURCES_PATH ${CMAKE_SOURCE_DIR}/res
    CACHE PATH "Location of the resources folder.")

set(THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty/BUILD")
set(INCLUDE_DIR "${THIRDPARTY_DIR}/include")
set(LIB_DIR "${THIRDPARTY_DIR}/lib")

set(EIGEN_INCLUDE_DIR_HINTS "${INCLUDE_DIR}")
set(SDL2_DIR_HINTS "${INCLUDE_DIR}/SDL" "${LIB_DIR}")
set(GLEW_DIR_HINTS "${THIRDPARTY_DIR}")

find_package(OpenGL REQUIRED)
find_package(Eigen REQUIRED)
find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)

add_subdirectory(src)
