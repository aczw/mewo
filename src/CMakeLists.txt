add_library(mewo STATIC
  core/gpu/string.cpp
  core/gpu/string.hpp
  core/gpu/types.hpp

  core/shader/assembler.cpp
  core/shader/assembler.hpp
  core/shader/uniform_data.cpp
  core/shader/uniform_data.hpp
  core/shader/uniforms.cpp
  core/shader/uniforms.hpp

  core/utility/match.hpp

  core/event.cpp
  core/event.hpp
  core/pinned.hpp

  impl/common/dimensions.hpp
  impl/common/string.cpp
  impl/common/string.hpp

  impl/project/pass.cpp
  impl/project/pass.hpp

  impl/session/pass_state.cpp
  impl/session/pass_state.hpp
  impl/session/pass_targets.cpp
  impl/session/pass_targets.hpp
  impl/session/surface_blit.cpp
  impl/session/surface_blit.hpp
  
  impl/surface/source.hpp

  impl/project.cpp
  impl/project.hpp
  impl/runtime.cpp
  impl/runtime.hpp
  impl/session.cpp
  impl/session.hpp
  impl/surface.cpp
  impl/surface.hpp

  project.cpp
  runtime.cpp
  session.cpp
  surface.cpp
  version.cpp
)
mewo_set_target_defaults(mewo CXX)
target_include_directories(mewo
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(mewo PRIVATE nlohmann_json::nlohmann_json webgpu)
