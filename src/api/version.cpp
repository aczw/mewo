#include <mewo/mewo.h>

extern "C" mewo_string_view_t mewo_get_version() {
  return {
    .data = MEWO_VERSION,
    .length = sizeof(MEWO_VERSION) - 1,
  };
}
