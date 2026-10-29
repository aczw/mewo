#ifndef MEWO_VERSION_H
#define MEWO_VERSION_H

#include <mewo/detail/types.h>

#ifdef __cplusplus
extern "C" {
#endif

mewo_string_view_t mewo_get_version(void);

#ifdef __cplusplus
}
#endif

#endif
