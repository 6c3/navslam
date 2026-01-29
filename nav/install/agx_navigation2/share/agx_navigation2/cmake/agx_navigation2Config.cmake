# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_agx_navigation2_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED agx_navigation2_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(agx_navigation2_FOUND FALSE)
  elseif(NOT agx_navigation2_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(agx_navigation2_FOUND FALSE)
  endif()
  return()
endif()
set(_agx_navigation2_CONFIG_INCLUDED TRUE)

# output package information
if(NOT agx_navigation2_FIND_QUIETLY)
  message(STATUS "Found agx_navigation2: 0.0.0 (${agx_navigation2_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'agx_navigation2' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${agx_navigation2_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(agx_navigation2_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${agx_navigation2_DIR}/${_extra}")
endforeach()
