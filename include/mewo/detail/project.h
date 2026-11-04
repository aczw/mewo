#ifndef MEWO_PROJECT_H
#define MEWO_PROJECT_H

#include <mewo/detail/pass.h>
#include <mewo/detail/types.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
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

mewo_result_t mewo_project_set_pass_source(
  mewo_project_t* project,
  mewo_pass_id_t pass_id,
  mewo_string_view_t source
);

/**
 * @param project Pointer to the project.
 * @return The number of passes currently in this project. A project always has at least
 * 1 pass, so the result is always >= 1.
 */
uint32_t mewo_project_get_pass_count(mewo_project_t* project);

/**
 * Retrieves the ID associated with a pass from a project. A project must always have
 * at least one pass, so passing 0 as the index will always return a valid pass ID.
 *
 * @param project Pointer to the project.
 * @param index Index of the pass in the list. Must be less than the length of the list.
 * @return The ID of the pass, or `MEWO_PASS_ID_NONE` if `index` is invalid.
 */
mewo_pass_id_t mewo_project_get_pass_id(mewo_project_t* project, uint32_t index);

#ifdef __cplusplus
}
#endif

#endif
