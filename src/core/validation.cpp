#include "core/validation.hpp"

#include <algorithm>
#include <array>
#include <string_view>

namespace mewo {
namespace {

constexpr std::array kReservedNames = std::to_array<std::string_view>({
  "CON",  "PRN",  "AUX",  "NUL",  "COM0", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7",
  "COM8", "COM9", "LPT0", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9",
});

bool is_ascii_digit(unsigned char character) { return character >= '0' && character <= '9'; }

bool is_ascii_upper(unsigned char character) { return character >= 'A' && character <= 'Z'; }

bool is_ascii_lower(unsigned char character) { return character >= 'a' && character <= 'z'; }

bool is_allowed_symbols(unsigned char character) {
  return character == ' ' || character == '(' || character == ')' || character == '+' ||
         character == '-' || character == '.' || character == '_';
}

unsigned char to_ascii_upper(unsigned char character) {
  return is_ascii_lower(character) ? character - ('a' - 'A') : character;
}

bool equals_ignore_case(std::string_view a, std::string_view b) {
  return std::ranges::equal(a, b, {}, to_ascii_upper, to_ascii_upper);
}

}  // namespace

bool is_valid_name(std::string_view name) {
  if (name.empty() || name.length() > 64)
    return false;

  if (!std::ranges::all_of(name, [](unsigned char character) {
        return is_ascii_digit(character) || is_ascii_upper(character) ||
               is_ascii_lower(character) || is_allowed_symbols(character);
      })) {
    return false;
  }

  if (name.starts_with(' ') || name.starts_with('.') || name.ends_with(' ') ||
      name.ends_with('.')) {
    return false;
  }

  // Windows matches device names on the part before the first dot, ignoring trailing spaces
  auto sub_view = name.substr(0, name.find('.'));
  auto stem = sub_view.substr(0, sub_view.find_last_not_of(' ') + 1);
  if (std::ranges::any_of(kReservedNames, [&stem](std::string_view reserved_name) {
        return equals_ignore_case(stem, reserved_name);
      })) {
    return false;
  }

  return true;
}

}  // namespace mewo
