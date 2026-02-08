cmake_minimum_required(VERSION 4.0.0)

project(mewo VERSION 0.0.1 LANGUAGES C CXX)

# Set up Dawn
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "") # Dawn dependencies requires Python
set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "") # Disables Dawn from installing binaries/headers
set(DAWN_USE_GLFW OFF CACHE BOOL "") # We're using SDL
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "")
set(DAWN_BUILD_TESTS OFF CACHE BOOL "")
set(DAWN_WERROR ON CACHE BOOL "")
add_subdirectory(third_party/dawn EXCLUDE_FROM_ALL)

add_executable(mewo src/main.cpp)

target_compile_features(mewo PRIVATE cxx_std_23)
target_compile_options(mewo PRIVATE -Wall -Wextra -Werror -Wpedantic -Wconversion)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(mewo PRIVATE -O2 -g)
  elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(FATAL_ERROR "TODO: add compile flags for release Clang builds")
  else()
    message(FATAL_ERROR "Unsupported CMAKE_BUILD_TYPE. Supported values are Debug and Release")
  endif()
else()
  message(FATAL_ERROR "Unsupported compiler used. Supported compilers are Clang (LLVM/Apple)")
endif()
set_target_properties(mewo PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

target_link_libraries(mewo PRIVATE dawn::webgpu_dawn)
