cmake_minimum_required(VERSION 4.0.0)

project(mewo VERSION 0.0.1 LANGUAGES C CXX)

# Set up Dawn
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "") # Dawn dependencies requires Python
set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "") # Disables Dawn from installing binaries/headers
set(DAWN_USE_GLFW OFF CACHE BOOL "") # We're using SDL
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "")
set(DAWN_BUILD_TESTS OFF CACHE BOOL "")
set(DAWN_WERROR ON CACHE BOOL "")
# Ask Dawn to look in system paths for dynamic libraries like libvulkan
set(DAWN_FORCE_SYSTEM_COMPONENT_LOAD ON CACHE BOOL "")
add_subdirectory(third_party/dawn EXCLUDE_FROM_ALL)

# Set up SDL, building it as a static library
set(SDL_SHARED OFF CACHE BOOL "")
set(SDL_STATIC ON CACHE BOOL "")
add_subdirectory(third_party/SDL EXCLUDE_FROM_ALL)

set(MEWO_IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

# Set up Dear ImGui as a set of compiled objects that are then linked against. Allows me
# to treat it as a separate library without actually being one 
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

  ${MEWO_IMGUI_DIR}/backends/imgui_impl_sdl3.h
  ${MEWO_IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
  ${MEWO_IMGUI_DIR}/backends/imgui_impl_wgpu.h
  ${MEWO_IMGUI_DIR}/backends/imgui_impl_wgpu.cpp

  ${MEWO_IMGUI_DIR}/misc/cpp/imgui_stdlib.h
  ${MEWO_IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
)

# imgui_impl_wgpu.cpp conditionally includes macOS-specific headers, so we need to
# treat it as a Obj-C file on macOS. On other OSes it's a regular C++ file.
if(APPLE)
  set_source_files_properties(${MEWO_IMGUI_DIR}/backends/imgui_impl_wgpu.cpp PROPERTIES
    # I do not enjoy hard coding compile flags like this, but this would only ever be
    # compiled on macOS, and both Clang and GCC (uhhhhhh on Apple???) support this flag
    COMPILE_FLAGS "--language objective-c++" 
  )
endif()

# Setting this definition to PUBLIC lets it propagate to Mewo when it links against it
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_WEBGPU_BACKEND_DAWN)
# Let Dear ImGui find its own files, and set it to PUBLIC so that it also propagates to Mewo
# when it links against the target. Also include the `backends/` folder so I don't have to prefix
# files in the folder with it when I #include them.
target_include_directories(imgui PUBLIC ${MEWO_IMGUI_DIR} ${MEWO_IMGUI_DIR}/backends) 
target_link_libraries(imgui PRIVATE dawn::webgpu_dawn SDL3::SDL3)

add_executable(mewo src/main.cpp)

# Common C++ compiler options
target_compile_features(mewo PRIVATE cxx_std_23)
target_compile_options(mewo PRIVATE 
  -Wall -Wextra -Werror -Wpedantic -Wconversion
  # Skipped fields are zero-initialized or default constructed, which I want
  -Wno-missing-designated-field-initializers
)

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

if(WIN32)
  target_compile_definitions(mewo PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
  )
endif()

set_target_properties(mewo PROPERTIES CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)

# Link to third-party libraries
target_link_libraries(mewo PRIVATE
  dawn::webgpu_dawn
  SDL3::SDL3
  imgui
)