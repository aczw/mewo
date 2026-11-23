#pragma once

#include <cstddef>
#include <optional>
#include <string_view>

namespace mewo {

inline constexpr std::size_t kMaxNameLength = 64;

/** Why a project or pass name was rejected. */
enum class NameError {
  kEmpty,
  /** Longer than `kMaxNameLength`. */
  kTooLong,
  /** Contains a character outside `A-Z`, `a-z`, `0-9`, space, `_`, `-`, `(`, `)`, `+`, `.` */
  kInvalidCharacter,
  /** Starts or ends with a space or `.` */
  kLeadingOrTrailing,
  /**
   * A Windows reserved device name (`CON`, `PRN`, `AUX`, `NUL`, `COM0`-`COM9`, `LPT0`-`LPT9`),
   * matched case-insensitively against the part before the first `.`, ignoring trailing spaces.
   */
  kReserved,
  /**
   * Another pass in the project already has this name, compared case-insensitively.
   */
  kTaken,
};

/**
 * Checks whether a project or pass name is valid. Names become file names, so they must be valid
 * on Windows, macOS, and Linux.
 *
 * @return Nothing if the name is valid, otherwise the first error found.
 */
std::optional<NameError> check_name(std::string_view name);

bool is_valid_name(std::string_view name);

/** Compares two strings, treating ASCII letters case-insensitively. */
bool equals_ignore_case(std::string_view a, std::string_view b);

}  // namespace mewo
