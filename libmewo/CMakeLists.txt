add_library(mewo_model STATIC
  src/project/project.cpp
  src/project/project.hpp
)
mewo_set_target_defaults(mewo_model CXX)
target_link_libraries(mewo_model PRIVATE nlohmann_json::nlohmann_json)
# Let the include directories propagate to later libraries
target_include_directories(mewo_model PUBLIC src)

add_library(mewo STATIC
  src/api/handles.hpp
  src/api/result.cpp
  src/api/result.hpp
  src/api/runtime.cpp
  src/api/version.cpp

  src/gpu/move_only_handle.hpp
  src/gpu/types.hpp

  src/runtime/runtime.hpp
  src/runtime/runtime.cpp

  src/utility/error.hpp
)
mewo_set_target_defaults(mewo CXX)
target_include_directories(mewo PUBLIC include PRIVATE src)
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
