#include "impl/project.hpp"

#include <mewo/mewo.h>

#include <cassert>

extern "C" {
mewo_result_t mewo_project_create(mewo_project_t** out) {
  assert(out);
  *out = nullptr;

  try {
    *out = new mewo_project_t(mewo::impl::Project::create());
  } catch (...) {
    return MEWO_ERROR_INTERNAL;
  }

  return MEWO_SUCCESS;
}

void mewo_project_destroy(mewo_project_t* project) { delete project; }
}
