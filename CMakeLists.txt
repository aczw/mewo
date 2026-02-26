cmake_minimum_required(VERSION 4.0.0)

set(MEWO_VERSION_SEMVER 0.1.0)
set(MEWO_VERSION_BUILD 0)
set(MEWO_VERSION_SUFFIX -alpha)
# Notably, the "full" version string does not include the build number
set(MEWO_VERSION_FULL "${MEWO_VERSION_SEMVER}${MEWO_VERSION_SUFFIX}")

project(mewo
  VERSION "${MEWO_VERSION_SEMVER}.${MEWO_VERSION_BUILD}"
  HOMEPAGE_URL https://mewo.czw.sh
  LANGUAGES C CXX
)

# Handle unsupported platforms up front
if(APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(MEWO_PLATFORM_MACOS ON)
elseif(WIN32)
  set(MEWO_PLATFORM_WINDOWS ON)
else()
  message(FATAL_ERROR "Unsupported OS. Supported platforms are macOS and Windows")
endif()

# Handle unsupported CMake build types up front
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(MEWO_BUILD_DEBUG ON)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(MEWO_BUILD_RELEASE ON)
else()
  message(FATAL_ERROR "Unsupported CMAKE_BUILD_TYPE. Supported values are Debug and Release")
endif()

set(MEWO_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Set up Dawn
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "") # Dawn dependencies requires Python
set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "") # Disables Dawn from installing binaries/headers
set(DAWN_USE_GLFW OFF CACHE BOOL "") # We're using SDL
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "")
set(DAWN_BUILD_TESTS OFF CACHE BOOL "")
set(DAWN_WERROR ON CACHE BOOL "")
# Ask Dawn to look in system paths for dynamic libraries like libvulkan
set(DAWN_FORCE_SYSTEM_COMPONENT_LOAD ON CACHE BOOL "")
add_subdirectory(${MEWO_THIRD_PARTY_DIR}/dawn EXCLUDE_FROM_ALL)

# Set up SDL, building it as a static library
set(SDL_SHARED OFF CACHE BOOL "")
set(SDL_STATIC ON CACHE BOOL "")
add_subdirectory(${MEWO_THIRD_PARTY_DIR}/SDL EXCLUDE_FROM_ALL)

# Set up FreeType, needed for Dear ImGui's FreeType backend
set(SKIP_INSTALL_ALL ON CACHE BOOL "")
set(FT_ENABLE_ERROR_STRINGS ON CACHE BOOL "")
set(FT_DYNAMIC_HARFBUZZ OFF CACHE BOOL "")
add_subdirectory(${MEWO_THIRD_PARTY_DIR}/freetype EXCLUDE_FROM_ALL)

# Set up Dear ImGui as a set of compiled objects that are then linked against. Allows me
# to treat it as a separate library without actually being one
set(MEWO_IMGUI_DIR ${MEWO_THIRD_PARTY_DIR}/imgui)
add_library(imgui OBJECT
  ${MEWO_IMGUI_DIR}/imconfig.h
  ${MEWO_IMGUI_DIR}/imgui_demo.cpp
  ${MEWO_IMGUI_DIR}/imgui_draw.cpp
  ${MEWO_IMGUI_DIR}/imgui_internal.h
  ${MEWO_IMGUI_DIR}/imgui_tables.cpp
  ${MEWO_IMGUI_DIR}/imgui_widgets.cpp
  ${MEWO_IMGUI_DIR}/imgui.cpp
  ${MEWO_IMGUI_DIR}/imgui.h
  ${MEWO_IMGUI_DIR}/imstb_rectpack.h
  ${MEWO_IMGUI_DIR}/imstb_textedit.h
  ${MEWO_IMGUI_DIR}/imstb_truetype.h

  ${MEWO_IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
  ${MEWO_IMGUI_DIR}/backends/imgui_impl_sdl3.h
  ${MEWO_IMGUI_DIR}/backends/imgui_impl_wgpu.cpp
  ${MEWO_IMGUI_DIR}/backends/imgui_impl_wgpu.h

  ${MEWO_IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
  ${MEWO_IMGUI_DIR}/misc/cpp/imgui_stdlib.h

  ${MEWO_IMGUI_DIR}/misc/freetype/imgui_freetype.cpp
  ${MEWO_IMGUI_DIR}/misc/freetype/imgui_freetype.h
)
# imgui_impl_wgpu.cpp conditionally includes macOS-specific headers, so we need to
# treat it as a Obj-C file on macOS. On other OSes it's a regular C++ file.
if(MEWO_PLATFORM_MACOS)
  set_source_files_properties(${MEWO_IMGUI_DIR}/backends/imgui_impl_wgpu.cpp PROPERTIES
    # I do not enjoy hard coding compile flags like this, but this would only ever be
    # compiled on macOS, and both Clang and GCC (uhhhhhh on Apple???) support this flag
    COMPILE_FLAGS "--language objective-c++" 
  )
endif()
# Setting this definition to PUBLIC lets it propagate to Mewo when it links against it
target_compile_definitions(imgui PUBLIC
  IMGUI_IMPL_WEBGPU_BACKEND_DAWN
  # Use our own imconfig.h file because I don't want to modify Git submodule
  IMGUI_USER_CONFIG="../../src/gui/imconfig.h"
)
# Let Dear ImGui find its own files, and set it to PUBLIC so that it also propagates to Mewo
# when it links against the target. Also include some subdirectories so I don't have to prefix
# files in the folder with it when I #include them.
target_include_directories(imgui PUBLIC
  ${MEWO_IMGUI_DIR}
  ${MEWO_IMGUI_DIR}/backends
  ${MEWO_IMGUI_DIR}/misc/cpp
)
target_link_libraries(imgui PRIVATE dawn::webgpu_dawn SDL3::SDL3 freetype)

set(MEWO_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(MEWO_GFX_DIR ${MEWO_SRC_DIR}/gfx)
set(MEWO_GUI_DIR ${MEWO_SRC_DIR}/gui)
set(MEWO_SDL_DIR ${MEWO_SRC_DIR}/sdl)

add_executable(mewo 
  ${MEWO_SDL_DIR}/context.cpp
  ${MEWO_SDL_DIR}/context.hpp
  ${MEWO_SDL_DIR}/window.cpp
  ${MEWO_SDL_DIR}/window.hpp

  ${MEWO_GFX_DIR}/create.cpp
  ${MEWO_GFX_DIR}/create.hpp
  ${MEWO_GFX_DIR}/error.hpp
  ${MEWO_GFX_DIR}/frame_context.hpp
  ${MEWO_GFX_DIR}/renderer.cpp
  ${MEWO_GFX_DIR}/renderer.hpp

  ${MEWO_GUI_DIR}/context.cpp
  ${MEWO_GUI_DIR}/context.hpp
  ${MEWO_GUI_DIR}/layout.cpp
  ${MEWO_GUI_DIR}/layout.hpp

  ${MEWO_SRC_DIR}/aspect_ratio.cpp
  ${MEWO_SRC_DIR}/aspect_ratio.hpp
  ${MEWO_SRC_DIR}/assets.cpp
  ${MEWO_SRC_DIR}/assets.hpp
  ${MEWO_SRC_DIR}/editor.cpp
  ${MEWO_SRC_DIR}/editor.hpp
  ${MEWO_SRC_DIR}/exception.cpp
  ${MEWO_SRC_DIR}/exception.hpp
  ${MEWO_SRC_DIR}/fs.cpp
  ${MEWO_SRC_DIR}/fs.hpp
  ${MEWO_SRC_DIR}/main.cpp
  ${MEWO_SRC_DIR}/mewo.cpp
  ${MEWO_SRC_DIR}/mewo.hpp
  ${MEWO_SRC_DIR}/query.hpp
  ${MEWO_SRC_DIR}/state.hpp
  ${MEWO_SRC_DIR}/utility.hpp
  ${MEWO_SRC_DIR}/viewport.cpp
  ${MEWO_SRC_DIR}/viewport.hpp
)

# So I can always include files starting from `src` instead of ugly relative paths
target_include_directories(mewo PRIVATE ${MEWO_SRC_DIR})

# Common C++ compiler options
target_compile_features(mewo PRIVATE cxx_std_23)
set_target_properties(mewo PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
target_compile_options(mewo PRIVATE 
  -Wall -Wextra -Werror -Wpedantic -Wconversion
  # Skipped fields are zero-initialized or default constructed, which I want
  -Wno-missing-designated-field-initializers
)
target_compile_definitions(mewo PRIVATE
  MEWO_VERSION_MAJOR="${PROJECT_VERSION_MAJOR}"
  MEWO_VERSION_MINOR="${PROJECT_VERSION_MINOR}"
  MEWO_VERSION_PATCH="${PROJECT_VERSION_PATCH}"
  MEWO_VERSION_BUILD="${PROJECT_VERSION_TWEAK}"
  MEWO_VERSION_SEMVER="${MEWO_VERSION_SEMVER}"
  MEWO_VERSION_SUFFIX="${MEWO_VERSION_SUFFIX}"
  MEWO_VERSION_FULL="${MEWO_VERSION_FULL}"
)

# Set compiler flags based on compiler and build type
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  if(MEWO_BUILD_DEBUG)
    target_compile_options(mewo PRIVATE -O2 -g)
  elseif(MEWO_BUILD_RELEASE)
    target_compile_options(mewo PRIVATE -O3)
    # Enables link-time optimization
    set_target_properties(mewo PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
  endif()
else()
  message(FATAL_ERROR "Unsupported compiler used. Supported compilers are Clang (LLVM/Apple)")
endif()

# Affects <windows.h>
if(MEWO_PLATFORM_WINDOWS)
  target_compile_definitions(mewo PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
endif()

if (MEWO_BUILD_DEBUG)
  # It's very difficult to detect if the program is in debug mode from the environment
  # so I use my own check for detection
  target_compile_definitions(mewo PRIVATE MEWO_IS_DEBUG)
endif()

# Link to third-party libraries
target_link_libraries(mewo PRIVATE
  dawn::webgpu_dawn
  SDL3::SDL3
  imgui
)

set(MEWO_DIST_README_FILE_PATH ${CMAKE_CURRENT_BINARY_DIR}/packaging/README.txt)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/packaging/README.txt.in
  ${MEWO_DIST_README_FILE_PATH}
  @ONLY
)

# Copy binary and assets folder for distribution
install(TARGETS mewo DESTINATION .)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION .)
install(FILES ${MEWO_DIST_README_FILE_PATH} DESTINATION .)

# Default system names are ugly (e.g. on macOS it's set to "Darwin")
if(MEWO_PLATFORM_MACOS)
  set(CPACK_SYSTEM_NAME "macOS")
elseif(MEWO_PLATFORM_WINDOWS)
  set(CPACK_SYSTEM_NAME "Win")
endif()

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "Mewo")
set(CPACK_PACKAGE_VENDOR "Charles Wang")
set(CPACK_PACKAGE_VERSION ${MEWO_VERSION_FULL})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-v${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist/${CMAKE_BUILD_TYPE}")

include(CPack)
