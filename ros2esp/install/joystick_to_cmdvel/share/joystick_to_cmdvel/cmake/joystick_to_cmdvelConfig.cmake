# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_joystick_to_cmdvel_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED joystick_to_cmdvel_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(joystick_to_cmdvel_FOUND FALSE)
  elseif(NOT joystick_to_cmdvel_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(joystick_to_cmdvel_FOUND FALSE)
  endif()
  return()
endif()
set(_joystick_to_cmdvel_CONFIG_INCLUDED TRUE)

# output package information
if(NOT joystick_to_cmdvel_FIND_QUIETLY)
  message(STATUS "Found joystick_to_cmdvel: 1.0.0 (${joystick_to_cmdvel_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'joystick_to_cmdvel' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${joystick_to_cmdvel_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(joystick_to_cmdvel_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${joystick_to_cmdvel_DIR}/${_extra}")
endforeach()
