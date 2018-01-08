#
# Try to find GLEW library and include path.
# Once done this will define:
#
# GLEW_FOUND
# GLEW_INCLUDE_DIR
# GLEW_LIBRARY
#
# You may set GLEW_DIR_HINTS to point to
# a folder containing the required files.
#

if(WIN32)
  find_path(GLEW_INCLUDE_DIR GL/glew.h
    ${GLEW_DIR_HINTS}/include/)

  find_library(GLEW_LIBRARY
    NAMES glew32.lib
    PATHS ${GLEW_DIR_HINTS}/lib)

elseif(APPLE)
  find_path(GLEW_INCLUDE_DIR GL/glew.h
    PATHS $ENV{GLEW_DIR_HINTS}/include/ /usr/include /usr/local/include /opt/local/include)

  find_library (GLEW_LIBRARY
    NAMES libGLEW.a
    PATHS $ENV{GLEW_DIR_HINTS}/lib /usr/lib /usr/local/lib /opt/local/lib)

else()
  find_path(GLEW_INCLUDE_DIR GL/glew.h)

  find_library(GLEW_LIBRARY
    NAMES GLEW glew32 glew glew32s PATH_SUFFIXES lib64)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW
  REQUIRED_VARS GLEW_INCLUDE_DIR GLEW_LIBRARY)
