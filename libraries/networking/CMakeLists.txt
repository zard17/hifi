cmake_minimum_required(VERSION 2.8)

if (WIN32)
  cmake_policy (SET CMP0020 NEW)
endif (WIN32)

set(ROOT_DIR ../..)
set(MACRO_DIR "${ROOT_DIR}/cmake/macros")

set(TARGET_NAME networking)
project(${TARGET_NAME})

find_package(Qt5 COMPONENTS Network)
find_package(GnuTLS REQUIRED)

include(${MACRO_DIR}/SetupHifiLibrary.cmake)
setup_hifi_library(${TARGET_NAME})

# include GLM
include(${MACRO_DIR}/IncludeGLM.cmake)
include_glm(${TARGET_NAME} "${ROOT_DIR}")

# add a definition for ssize_t so that windows doesn't bail on gnutls.h
if (WIN32)
  add_definitions(-Dssize_t=long)
endif ()

include_directories(SYSTEM "${GNUTLS_INCLUDE_DIR}")
target_link_libraries(${TARGET_NAME} Qt5::Network "${GNUTLS_LIBRARY}")