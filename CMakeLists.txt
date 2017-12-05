cmake_minimum_required(VERSION 3.5)

project(ansimproj VERSION 0.0.1)
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED 14)

set(RESOURCES_PATH ${CMAKE_SOURCE_DIR}/res
    CACHE PATH "Location of the resources folder.")

# TODO: resolve dependencies on linux
find_package(OpenGL REQUIRED)
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/BUILD/include")
link_directories("${CMAKE_SOURCE_DIR}/3rdparty/BUILD/lib")

if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-reorder")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall /WX")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")
endif()

add_subdirectory(src)
