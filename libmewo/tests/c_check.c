#include <mewo/mewo.h>

_Static_assert(sizeof(((mewo_event_t*)0)->variant) == 32, "event payload outgrew reserved space");
