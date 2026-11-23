#include "core/validation.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

using mewo::check_name;
using mewo::is_valid_name;
using mewo::kMaxNameLength;
using mewo::NameError;

TEST_CASE("check_name") {
  SECTION("accepts ordinary names") {
    CHECK_FALSE(check_name("scene"));
    CHECK_FALSE(check_name("Blur 2"));
    CHECK_FALSE(check_name("bloom (wide)"));
    CHECK_FALSE(check_name("tone+map"));
    CHECK_FALSE(check_name("blur.v2"));
    CHECK_FALSE(check_name("snake_case-name"));
    CHECK_FALSE(check_name("a"));
    CHECK_FALSE(check_name("0"));
  }

  SECTION("enforces the length limit") {
    CHECK(check_name("") == NameError::kEmpty);
    CHECK_FALSE(check_name(std::string(kMaxNameLength, 'a')));
    CHECK(check_name(std::string(kMaxNameLength + 1, 'a')) == NameError::kTooLong);
  }

  SECTION("rejects characters outside the allowed set") {
    for (const char* name : {
           "a/b", "a\\b", "a:b", "a*b", "a?b", "a\"b", "a<b>", "a|b", "a,b", "a=b", "a@b", "a#b",
           "a%b", "a&b", "a'b", "a~b", "a\tb", "a\nb",
           "caf\xC3\xA9",  // "café" in UTF-8
         }) {
      INFO("name: " << name);
      CHECK(check_name(name) == NameError::kInvalidCharacter);
    }

    CHECK(check_name(std::string("a\0b", 3)) == NameError::kInvalidCharacter);
  }

  SECTION("rejects a leading or trailing space or dot") {
    for (const char* name : {" blur", "blur ", ".hidden", "trailing.", " ", ".", ".."}) {
      INFO("name: " << name);
      CHECK(check_name(name) == NameError::kLeadingOrTrailing);
    }
  }

  SECTION("rejects Windows reserved names, ignoring case") {
    for (const char* name :
         {"CON", "con", "Nul", "prn", "aux", "com0", "com8", "COM9", "lpt0", "LPT9"}) {
      INFO("name: " << name);
      CHECK(check_name(name) == NameError::kReserved);
    }
  }

  SECTION("checks reserved names against the part before the first dot") {
    for (const char* name : {"con.v2", "nul.blur.v3", "con .v2", "con   .v2"}) {
      INFO("name: " << name);
      CHECK(check_name(name) == NameError::kReserved);
    }
  }

  SECTION("accepts names that only resemble reserved names") {
    for (const char* name : {"console", "null", "com10", "lpt", "blur.con", "my con", "con_v2"}) {
      INFO("name: " << name);
      CHECK_FALSE(check_name(name));
    }
  }

  SECTION("reports only the first error, in declaration order") {
    // Too long and full of invalid characters: length is checked first
    CHECK(check_name(std::string(kMaxNameLength + 1, '/')) == NameError::kTooLong);
    // Invalid character and a trailing dot: characters are checked first
    CHECK(check_name("a/b.") == NameError::kInvalidCharacter);
    // Reserved and a trailing space: leading/trailing is checked first
    CHECK(check_name("con ") == NameError::kLeadingOrTrailing);
  }
}

TEST_CASE("is_valid_name matches check_name") {
  CHECK(is_valid_name("scene"));
  CHECK_FALSE(is_valid_name(""));
  CHECK_FALSE(is_valid_name("a/b"));
  CHECK_FALSE(is_valid_name("con"));
}

}  // namespace
