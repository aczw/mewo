add_library(webgpu INTERFACE)

if (MEWO_WEBGPU_IMPL STREQUAL "dawn")
  target_link_libraries(webgpu INTERFACE dawn::webgpu_dawn)
else()
  message(FATAL_ERROR "Unknown WebGPU implementation: ${MEWO_WEBGPU_IMPL}")
endif()
