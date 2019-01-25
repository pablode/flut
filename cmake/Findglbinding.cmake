#
# Find glbinding
#
# Once done, this will define:
# glbinding_FOUND
# glbinding_INCLUDE_DIRS
# glbinding_LIBRARIES
#

find_path(glbinding_INCLUDE_DIRS
  NAMES glbinding/gl/gl.h glbinding/Binding.h
  PATHS /usr /usr/local
  PATH_SUFFIXES include)

find_library(glbinding_LIBRARY_RELEASE
  NAMES glbinding libglbinding
  PATHS /usr/lib /usr/local/lib)

find_library(glbinding_LIBRARY_DEBUG
  NAMES glbindingd libglbindingd
  PATHS /usr/lib /usr/local/lib)

include(SelectLibraryConfigurations)
select_library_configurations(glbinding)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(glbinding
  REQUIRED_VARS
  glbinding_INCLUDE_DIRS
  glbinding_LIBRARIES)

if(glbinding_FOUND AND NOT TARGET glbinding::glbinding)
    add_library(glbinding::glbinding UNKNOWN IMPORTED)

    set_target_properties(glbinding::glbinding PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${glbinding_INCLUDE_DIRS}")

    if (glbinding_LIBRARY_DEBUG)
      set_property(TARGET glbinding::glbinding APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(glbinding::glbinding PROPERTIES
        IMPORTED_LOCATION "${glbinding_LIBRARY_DEBUG}"
        IMPORTED_LOCATION_DEBUG "${glbinding_LIBRARY_DEBUG}")
    endif()

    if (glbinding_LIBRARY_RELEASE)
      set_property(TARGET glbinding::glbinding APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(glbinding::glbinding PROPERTIES
        IMPORTED_LOCATION "${glbinding_LIBRARY_RELEASE}"
        IMPORTED_LOCATION_RELEASE "${glbinding_LIBRARY_RELEASE}")
    endif()
endif()
