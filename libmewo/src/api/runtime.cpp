#include <mewo/mewo.h>

extern "C" mewo_result_t mewo_runtime_create(mewo_runtime_t** out) {
  (void)out;
  return MEWO_SUCCESS;
}

extern "C" void mewo_runtime_destroy(mewo_runtime_t* runtime) { (void)runtime; }
