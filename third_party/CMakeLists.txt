#
# Set up and add Dawn
#
# Dawn dependencies requires Python
set(DAWN_FETCH_DEPENDENCIES ON CACHE BOOL "")
# Disables Dawn from installing binaries/headers
set(DAWN_ENABLE_INSTALL OFF CACHE BOOL "")
set(DAWN_USE_GLFW OFF CACHE BOOL "")
set(DAWN_BUILD_SAMPLES OFF CACHE BOOL "")
set(DAWN_BUILD_TESTS OFF CACHE BOOL "")
set(DAWN_WERROR OFF CACHE BOOL "")
# Ask Dawn to look in system paths for dynamic libraries
set(DAWN_FORCE_SYSTEM_COMPONENT_LOAD ON CACHE BOOL "")
set(TINT_BUILD_TESTS OFF CACHE BOOL "")
add_subdirectory(dawn EXCLUDE_FROM_ALL SYSTEM)

#
# Set up and add SDL
#
if(MEWO_BUILD_HOSTS)
  # Build as a static library
  set(SDL_SHARED OFF CACHE BOOL "")
  set(SDL_STATIC ON CACHE BOOL "")
  add_subdirectory(SDL EXCLUDE_FROM_ALL SYSTEM)
endif()

#
# Set up and add nlohmann/json
#
set(JSON_BuildTests OFF CACHE BOOL "")
set(JSON_Install OFF CACHE BOOL "")
add_subdirectory(json EXCLUDE_FROM_ALL SYSTEM)

#
# Add Catch2
#
if(MEWO_BUILD_TESTS)
  add_subdirectory(Catch2 EXCLUDE_FROM_ALL SYSTEM)
endif()
