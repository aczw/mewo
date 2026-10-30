#ifndef MEWO_PROJECT_H
#define MEWO_PROJECT_H

#include <mewo/detail/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates and allocates a new project. On failure, `*out` is set to `NULL`.
 *
 * @param out Out parameter where a pointer to the project will be stored.
 * @return Returns whether the operation was successful.
 */
mewo_result_t mewo_project_create(mewo_project_t** out);

/**
 * Destroys the project. All sessions launched from this project must be destroyed first.
 *
 * @param project Pointer to the project. If `NULL` is passed, nothing happens.
 */
void mewo_project_destroy(mewo_project_t* project);

#ifdef __cplusplus
}
#endif

#endif
