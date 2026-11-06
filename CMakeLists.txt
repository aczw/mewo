cmake_minimum_required(VERSION 4.0.0)

set(MEWO_VERSION_SEMVER 1.0.0)
set(MEWO_VERSION_BUILD 0)

project(mewo
  VERSION "${MEWO_VERSION_SEMVER}"
  HOMEPAGE_URL "https://mewo.czw.sh"
  LANGUAGES C CXX
)

# Support Clang only
if(NOT (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
  message(FATAL_ERROR "Unsupported compiler used: ${CMAKE_CXX_COMPILER_ID}")
endif()

include(CMakeDependentOption)

if(EMSCRIPTEN)
  set(MEWO_DEFAULT_WEBGPU_IMPL "emdawnwebgpu")
else()
  set(MEWO_DEFAULT_WEBGPU_IMPL "dawn")
endif()

set(MEWO_WEBGPU_IMPL "${MEWO_DEFAULT_WEBGPU_IMPL}" CACHE STRING "WebGPU implementation")
set_property(CACHE MEWO_WEBGPU_IMPL PROPERTY STRINGS "dawn" "emdawnwebgpu")
option(MEWO_BUILD_TESTS "Build tests" ON)
cmake_dependent_option(MEWO_BUILD_HOST_SDL "Build SDL reference" ON "NOT EMSCRIPTEN" OFF)
cmake_dependent_option(MEWO_BUILD_HOST_WEB "Build WASM frontend" ON "EMSCRIPTEN" OFF)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(MEWO_BUILD_TESTS)
  enable_testing()
  # Necessary to call `catch_discover_tests()` later
  list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/third_party/Catch2/extras")
endif()

include(cmake/utils.cmake)
add_subdirectory(third_party)
include(cmake/webgpu.cmake)

add_subdirectory(src)

if(MEWO_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(MEWO_BUILD_HOST_SDL)
  add_subdirectory(hosts/sdl)
endif()

if(MEWO_BUILD_HOST_WEB)
  add_subdirectory(hosts/web/bindings)
endif()
