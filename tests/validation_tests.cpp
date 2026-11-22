#include "core/validation.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>

namespace {

using mewo::is_valid_name;

TEST_CASE("is_valid_name") {
  SECTION("accepts ordinary names") {
    CHECK(is_valid_name("scene"));
    CHECK(is_valid_name("Blur 2"));
    CHECK(is_valid_name("bloom (wide)"));
    CHECK(is_valid_name("tone+map"));
    CHECK(is_valid_name("blur.v2"));
    CHECK(is_valid_name("snake_case-name"));
    CHECK(is_valid_name("a"));
    CHECK(is_valid_name("0"));
  }

  SECTION("enforces the length limit") {
    CHECK_FALSE(is_valid_name(""));
    CHECK(is_valid_name(std::string(64, 'a')));
    CHECK_FALSE(is_valid_name(std::string(65, 'a')));
  }

  SECTION("rejects characters outside the allowed set") {
    CHECK_FALSE(is_valid_name("a/b"));
    CHECK_FALSE(is_valid_name("a\\b"));
    CHECK_FALSE(is_valid_name("a:b"));
    CHECK_FALSE(is_valid_name("a*b"));
    CHECK_FALSE(is_valid_name("a?b"));
    CHECK_FALSE(is_valid_name("a\"b"));
    CHECK_FALSE(is_valid_name("a<b>"));
    CHECK_FALSE(is_valid_name("a|b"));
    CHECK_FALSE(is_valid_name("a,b"));
    CHECK_FALSE(is_valid_name("a=b"));
    CHECK_FALSE(is_valid_name("a@b"));
    CHECK_FALSE(is_valid_name("a#b"));
    CHECK_FALSE(is_valid_name("a%b"));
    CHECK_FALSE(is_valid_name("a&b"));
    CHECK_FALSE(is_valid_name("a'b"));
    CHECK_FALSE(is_valid_name("a~b"));
    CHECK_FALSE(is_valid_name("a\tb"));
    CHECK_FALSE(is_valid_name("a\nb"));
    CHECK_FALSE(is_valid_name("caf\xC3\xA9"));  // "café" in UTF-8
    CHECK_FALSE(is_valid_name(std::string("a\0b", 3)));
  }

  SECTION("rejects a leading or trailing space or dot") {
    CHECK_FALSE(is_valid_name(" blur"));
    CHECK_FALSE(is_valid_name("blur "));
    CHECK_FALSE(is_valid_name(".hidden"));
    CHECK_FALSE(is_valid_name("trailing."));
    CHECK_FALSE(is_valid_name(" "));
    CHECK_FALSE(is_valid_name("."));
    CHECK_FALSE(is_valid_name(".."));
  }

  SECTION("rejects Windows reserved names, ignoring case") {
    CHECK_FALSE(is_valid_name("CON"));
    CHECK_FALSE(is_valid_name("con"));
    CHECK_FALSE(is_valid_name("Nul"));
    CHECK_FALSE(is_valid_name("prn"));
    CHECK_FALSE(is_valid_name("aux"));
    CHECK_FALSE(is_valid_name("com0"));
    CHECK_FALSE(is_valid_name("com8"));
    CHECK_FALSE(is_valid_name("COM9"));
    CHECK_FALSE(is_valid_name("lpt0"));
    CHECK_FALSE(is_valid_name("LPT9"));
  }

  SECTION("checks reserved names against the part before the first dot") {
    CHECK_FALSE(is_valid_name("con.v2"));
    CHECK_FALSE(is_valid_name("nul.blur.v3"));
    CHECK_FALSE(is_valid_name("con .v2"));
    CHECK_FALSE(is_valid_name("con   .v2"));
  }

  SECTION("accepts names that only resemble reserved names") {
    CHECK(is_valid_name("console"));
    CHECK(is_valid_name("null"));
    CHECK(is_valid_name("com10"));
    CHECK(is_valid_name("lpt"));
    CHECK(is_valid_name("blur.con"));
    CHECK(is_valid_name("my con"));
    CHECK(is_valid_name("con_v2"));
  }
}

}  // namespace
