#include "core/shader/assembler.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace {

using Kind = mewo::RelativeLine::Kind;

constexpr std::string_view kPassSource = "fn main(frag_coord: vec2f) -> vec4f { return helper(); }";
constexpr std::string_view kCommonSource = "fn helper() -> vec4f { return vec4f(1.0); }";

/**
 * Every segment ends in a '\n' added by the assembler, so an assembled module has exactly as many
 * lines as newlines.
 */
uint32_t count_lines(std::string_view code) {
  return static_cast<uint32_t>(std::ranges::count(code, '\n'));
}

/**
 * Returns the text of 1-based line `number` in `text`, without its '\n'. Empty if out of range.
 */
std::string_view line_at(std::string_view text, uint32_t number) {
  for (uint32_t line = 1; line < number; ++line) {
    const auto newline = text.find('\n');
    if (newline == std::string_view::npos) {
      return {};
    }
    text.remove_prefix(newline + 1);
  }
  return text.substr(0, text.find('\n'));
}

/**
 * Checks that module line `line` maps to line `number` of the segment `kind`.
 */
void check_line(const mewo::SegmentMap& segments, uint32_t line, Kind kind, uint32_t number) {
  INFO("module line " << line);
  const std::optional<mewo::RelativeLine> relative = segments.find_relative_line(line);
  REQUIRE(relative.has_value());
  CHECK(relative->kind == kind);
  CHECK(relative->number == number);
}

}  // namespace

TEST_CASE("assembled module orders pass, then Common, then generated code", "[shader][assembler]") {
  const std::string code = mewo::assemble_pass_source(kPassSource, kCommonSource).code;

  const auto pass_pos = code.find(kPassSource);
  const auto common_pos = code.find(kCommonSource);
  const auto entry_pos = code.find(mewo::kFragmentEntryPoint);

  REQUIRE(pass_pos != std::string::npos);
  REQUIRE(common_pos != std::string::npos);
  REQUIRE(entry_pos != std::string::npos);

  // The pass comes first so its line numbers need no mapping.
  CHECK(pass_pos == 0);
  CHECK(pass_pos < common_pos);
  CHECK(common_pos < entry_pos);
}

TEST_CASE("pass and Common sources are kept on separate lines", "[shader][assembler]") {
  // Neither source ends in a newline; the assembler must not glue them together
  // (a trailing `// comment` in the pass would otherwise swallow Common's first line).
  const std::string code = mewo::assemble_pass_source("// pass comment", "fn helper() {}").code;

  CHECK(code.find("// pass comment\nfn helper() {}\n") == 0);
}

TEST_CASE("Common source starts on the line after the pass source", "[shader][assembler]") {
  const std::string code = mewo::assemble_pass_source("a\nb\nc", "common").code;

  CHECK(code.starts_with("a\nb\nc\ncommon\n"));
}

TEST_CASE("empty Common source still assembles", "[shader][assembler]") {
  const std::string code = mewo::assemble_pass_source(kPassSource, "").code;

  CHECK(code.starts_with(kPassSource));
  CHECK(code.find(mewo::kFragmentEntryPoint) != std::string::npos);
  CHECK(code.find(mewo::kVertexEntryPoint) != std::string::npos);
}

TEST_CASE("assembled module ends in a newline", "[shader][assembler]") {
  // `count_lines` (and the segment map's line arithmetic) relies on this.
  const std::string code = mewo::assemble_pass_source(kPassSource, kCommonSource).code;

  CHECK(code.ends_with('\n'));
}

TEST_CASE("segment map: pass lines map to themselves", "[shader][assembler][segments]") {
  const auto assembled = mewo::assemble_pass_source("a\nb\nc", "x\ny");

  check_line(assembled.segments, 1, Kind::kPass, 1);
  check_line(assembled.segments, 2, Kind::kPass, 2);
  check_line(assembled.segments, 3, Kind::kPass, 3);
}

TEST_CASE("segment map: Common lines are 1-based within Common", "[shader][assembler][segments]") {
  const auto assembled = mewo::assemble_pass_source("a\nb\nc", "x\ny");

  check_line(assembled.segments, 4, Kind::kCommon, 1);
  check_line(assembled.segments, 5, Kind::kCommon, 2);
}

TEST_CASE("segment map: prelude lines are 1-based within the prelude", "[shader][assembler][segments]") {
  const auto assembled = mewo::assemble_pass_source("a\nb\nc", "x\ny");
  const uint32_t last_line = count_lines(assembled.code);

  check_line(assembled.segments, 6, Kind::kPrelude, 1);
  check_line(assembled.segments, last_line, Kind::kPrelude, last_line - 5);
}

TEST_CASE("segment map: pass source ending in a newline", "[shader][assembler][segments]") {
  // "a\n" plus the assembler's separator gives the pass a blank second line (an editor shows it as
  // line 2 too), so Common starts on line 3.
  const auto assembled = mewo::assemble_pass_source("a\n", "common");

  check_line(assembled.segments, 2, Kind::kPass, 2);
  check_line(assembled.segments, 3, Kind::kCommon, 1);
}

TEST_CASE("segment map: empty Common is a single line", "[shader][assembler][segments]") {
  const auto assembled = mewo::assemble_pass_source("a", "");

  check_line(assembled.segments, 2, Kind::kCommon, 1);
  check_line(assembled.segments, 3, Kind::kPrelude, 1);
}

TEST_CASE("segment map: lines outside the module have no position", "[shader][assembler][segments]") {
  const auto assembled = mewo::assemble_pass_source(kPassSource, kCommonSource);
  const uint32_t line_count = count_lines(assembled.code);

  // WebGPU reports line 0 for compilation messages without a source position.
  CHECK_FALSE(assembled.segments.find_relative_line(0).has_value());
  CHECK(assembled.segments.find_relative_line(line_count).has_value());
  CHECK_FALSE(assembled.segments.find_relative_line(line_count + 1).has_value());
}

TEST_CASE("segment map: every line maps back to its source text", "[shader][assembler][segments]") {
  constexpr std::string_view kPass =
    "fn main(frag_coord: vec2f) -> vec4f {\n  return helper();\n}\n";
  constexpr std::string_view kCommon = "\nfn helper() -> vec4f {\n  return vec4f(1.0);\n}";

  const auto assembled = mewo::assemble_pass_source(kPass, kCommon);
  const uint32_t line_count = count_lines(assembled.code);

  bool saw_pass = false;
  bool saw_common = false;
  bool saw_prelude = false;

  for (uint32_t line = 1; line <= line_count; ++line) {
    INFO("module line " << line);
    const auto relative = assembled.segments.find_relative_line(line);
    REQUIRE(relative.has_value());
    const std::string_view text = line_at(assembled.code, line);

    switch (relative->kind) {
      case Kind::kPass:
        saw_pass = true;
        CHECK(text == line_at(kPass, relative->number));
        break;
      case Kind::kCommon:
        saw_common = true;
        CHECK(text == line_at(kCommon, relative->number));
        break;
      case Kind::kPrelude:
        if (!saw_prelude) {
          // The generated code starts with the uniform struct.
          CHECK(relative->number == 1);
          CHECK(text.starts_with("struct mw_Uniforms"));
        }
        saw_prelude = true;
        break;
    }
  }

  CHECK(saw_pass);
  CHECK(saw_common);
  CHECK(saw_prelude);
  // The last line is the closing brace of the generated fragment entry point.
  CHECK(line_at(assembled.code, line_count) == "}");
}
