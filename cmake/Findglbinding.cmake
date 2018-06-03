#
# Find glbinding
#
# Once done, this will define:
# glbinding_FOUND
# glbinding_INCLUDE_DIRS
# glbinding_LIBRARIES
#

find_path(glbinding_INCLUDE_DIR
  NAMES glbinding/gl/gl.h glbinding/Binding.h
  PATHS /usr /usr/local
  PATH_SUFFIXES include)
set(glbinding_INCLUDE_DIRS ${glbinding_INCLUDE_DIR})

find_library(glbinding_LIBRARY
  NAMES glbinding libglbinding
  PATHS /usr/lib /usr/local/lib)
set(glbinding_LIBRARIES ${glbinding_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glbinding
  DEFAULT_MSG
  glbinding_INCLUDE_DIRS
  glbinding_LIBRARIES)

mark_as_advanced(glbinding_INCLUDE_DIRS glbinding_LIBRARIES)

if(glbinding_FOUND AND NOT TARGET glbinding::glbinding)
  add_library(glbinding::glbinding UNKNOWN IMPORTED)
  set_target_properties(glbinding::glbinding PROPERTIES
    IMPORTED_LOCATION "${glbinding_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${glbinding_INCLUDE_DIRS}")
endif()
