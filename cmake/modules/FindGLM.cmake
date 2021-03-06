# FindGLM - attempts to locate the glm matrix/vector library.
#
# This module defines the following variables (on success):
#   GLM_INCLUDE_DIRS  - where to find glm/glm.hpp
#   GLM_FOUND         - if the library was successfully located
#
# It is trying a few standard installation locations, but can be customized
# with the following variables:
#   GLM_ROOT_DIR      - root directory of a glm installation
#                       Headers are expected to be found in either:
#                       <GLM_ROOT_DIR>/glm/glm.hpp           OR
#                       <GLM_ROOT_DIR>/include/glm/glm.hpp
#                       This variable can either be a cmake or environment
#                       variable. Note however that changing the value
#                       of the environment varible will NOT result in
#                       re-running the header search and therefore NOT
#                       adjust the variables set by this module.

# This is a modified version of the FindGLM module included with CMake.
# Copyright 2014 High Fidelity
# 
#=============================================================================
# Copyright 2012 Carsten Neumann
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (GLM_INCLUDE_DIR)
  set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}")
  set(GLM_FOUND TRUE)
else ()
  # default search dirs
  set(_glm_HEADER_SEARCH_DIRS "$ENV{HIFI_LIB_DIR}/glm" "/usr/include" "/usr/local/include")

  # check environment variable
  set(_glm_ENV_ROOT_DIR "$ENV{GLM_ROOT_DIR}")

  if (NOT GLM_ROOT_DIR AND _glm_ENV_ROOT_DIR)
      set(GLM_ROOT_DIR "${_glm_ENV_ROOT_DIR}")
  endif ()

  # put user specified location at beginning of search
  if (GLM_ROOT_DIR)
      set(_glm_HEADER_SEARCH_DIRS "${GLM_ROOT_DIR}" "${GLM_ROOT_DIR}/include" "$ENV{HIFI_LIB_DIR}/glm" ${_glm_HEADER_SEARCH_DIRS})
  ENDIF()

  # locate header
  find_path(GLM_INCLUDE_DIR "glm/glm.hpp" PATHS ${_glm_HEADER_SEARCH_DIRS})

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(GLM DEFAULT_MSG GLM_INCLUDE_DIR)

  if (GLM_FOUND)
      set(GLM_INCLUDE_DIRS "${GLM_INCLUDE_DIR}")
    
      if (NOT GLM_FIND_QUIETLY)
        MESSAGE(STATUS "GLM_INCLUDE_DIR = ${GLM_INCLUDE_DIR}")
      endif (NOT GLM_FIND_QUIETLY)
  endif ()
endif () 