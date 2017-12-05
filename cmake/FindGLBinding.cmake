# - Try to find glbinding library
# Once done this will define
#
# GLBINDING_FOUND
# GLBINDING_INCLUDE_DIRS
# GLBINDING_LIBRARIES
# GLBINDING_DEFINITIONS
#

find_package(PkgConfig)
pkg_check_modules(PC_GLBINDING QUIET glbinding)
set(GLBINDING_DEFINITIONS ${PC_GLBINDING_CFLAGS_OTHER})

if(CMAKE_CL_64)
  set(PROGRAMFILES $ENV{ProgramW6432})
else()
  set(PROGRAMFILES $ENV{ProgramFiles})
endif()

find_path(GLBINDING_INCLUDE_DIR NAMES glbinding/gl/gl.h glbinding/Binding.h
          HINTS ${GLBINDING_DIR_HINTS}
          PATHS ${PROGRAMFILES}/glbinding/include)

find_library(GLBINDING_LIBRARY glbinding
             HINTS ${GLBINDING_DIR_HINTS}
             PATHS ${PROGRAMFILES}/glbinding/lib)

set(GLBINDING_LIBRARIES ${GLBINDING_LIBRARY} )
set(GLBINDING_INCLUDE_DIRS ${GLBINDING_INCLUDE_DIR} )

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set GLBINDING_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GLBINDING  DEFAULT_MSG
                                  GLBINDING_LIBRARY GLBINDING_INCLUDE_DIR)

mark_as_advanced(GLBINDING_INCLUDE_DIR GLBINDING_LIBRARY)
