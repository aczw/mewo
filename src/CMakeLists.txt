add_library(mewo STATIC
  core/gpu/string.cpp
  core/gpu/string.hpp
  core/gpu/types.hpp

  core/utility/match.hpp

  core/event.cpp
  core/event.hpp

  impl/surface/source.hpp

  impl/project.cpp
  impl/project.hpp
  impl/runtime.cpp
  impl/runtime.hpp
  impl/surface.cpp
  impl/surface.hpp

  project.cpp
  runtime.cpp
  surface.cpp
  version.cpp
)
mewo_set_target_defaults(mewo CXX)
target_include_directories(mewo
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(mewo PRIVATE nlohmann_json::nlohmann_json webgpu)
