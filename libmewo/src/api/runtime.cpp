#include "runtime/runtime.hpp"

#include "api/handles.hpp"
#include "api/result.hpp"

#include <mewo/mewo.h>

#include <cassert>
#include <utility>

extern "C" {
mewo_result_t mewo_runtime_create(mewo_runtime_t** out) {
  assert(out);
  *out = nullptr;

  try {
    auto runtime_res = mewo::Runtime::create();

    if (!runtime_res)
      return mewo::error_to_result(runtime_res.error());

    *out = new mewo_runtime(std::move(runtime_res.value()));
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

void mewo_runtime_destroy(mewo_runtime_t* runtime) { delete runtime; }

void mewo_runtime_tick(mewo_runtime_t* runtime) { assert(runtime); }

void mewo_runtime_request_device(mewo_runtime_t* runtime) {
  assert(runtime);
  runtime->impl->request_device();
}
}
