add_library(mewo_model STATIC
  project/project.cpp
  project/project.hpp
)
mewo_set_target_defaults(mewo_model CXX)
target_link_libraries(mewo_model PRIVATE nlohmann_json::nlohmann_json)
# Let the include directories propagate to later libraries
target_include_directories(mewo_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(mewo STATIC
  api/handles.hpp
  api/result.cpp
  api/result.hpp
  api/runtime.cpp
  api/surface.cpp
  api/version.cpp

  gpu/string.cpp
  gpu/string.hpp
  gpu/surface_source.cpp
  gpu/surface_source.hpp
  gpu/types.hpp

  core/error.hpp
  core/event.cpp
  core/event.hpp
  core/runtime.cpp
  core/runtime.hpp
  core/surface_state.cpp
  core/surface_state.hpp

  utility/match.hpp
)
mewo_set_target_defaults(mewo CXX)
target_include_directories(mewo
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(mewo PRIVATE mewo_model webgpu)

if(MEWO_BUILD_TESTS)
  add_executable(mewo_tests
    tests/project_tests.cpp
  )
  mewo_set_target_defaults(mewo_tests CXX)
  target_link_libraries(mewo_tests PRIVATE mewo_model Catch2::Catch2WithMain)

  add_library(mewo_c_check OBJECT
    tests/c_check.c
  )
  mewo_set_target_defaults(mewo_c_check C)
  target_link_libraries(mewo_c_check PRIVATE mewo)

  include(Catch)
  catch_discover_tests(mewo_tests)
endif()
