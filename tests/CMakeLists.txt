add_executable(mewo_tests
  project_tests.cpp
)
mewo_set_target_defaults(mewo_tests CXX)
# Tests can include internal headers directly
target_include_directories(mewo_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(mewo_tests PRIVATE mewo Catch2::Catch2WithMain)

add_library(mewo_c_check OBJECT
  c_check.c
)
mewo_set_target_defaults(mewo_c_check C)
target_link_libraries(mewo_c_check PRIVATE mewo)

include(Catch)
catch_discover_tests(mewo_tests)
