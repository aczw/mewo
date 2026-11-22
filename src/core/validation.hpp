#pragma once

#include <string_view>

namespace mewo {

/**
 * Checks whether a project or pass name is valid.
 *
 * - 1 to 64 characters.
 * - Only `A-Z`, `a-z`, `0-9`, space, `_`, `-`, `()`, `+`, and `.`
 * - The first and last characters are neither a space nor `.`
 * - Reserved names are not allowed on Windows. These include `CON`, `PRN`, `AUX`, `NUL`,
 * `COM0`-`COM9`, and `LPT0`-`LPT9`. They are matched case-insensitively against
 * the part before the first `.`.
 */
bool is_valid_name(std::string_view name);

/** Compares two strings, treating ASCII letters case-insensitively. */
bool equals_ignore_case(std::string_view a, std::string_view b);

}  // namespace mewo
