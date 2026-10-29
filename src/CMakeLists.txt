add_library(mewo STATIC
  api/runtime.cpp
  api/surface.cpp
  api/types.hpp
  api/version.cpp

  core/utility/match.hpp
  core/utility/wgpu_string.cpp
  core/utility/wgpu_string.hpp
  core/event.cpp
  core/event.hpp
  core/gpu_types.hpp

  project_impl.cpp
  project_impl.hpp
  runtime_impl.cpp
  runtime_impl.hpp
  surface_impl.cpp
  surface_impl.hpp
)
mewo_set_target_defaults(mewo CXX)
target_include_directories(mewo
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(mewo PRIVATE nlohmann_json::nlohmann_json webgpu)
