#include "core/shader/assembler.hpp"

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>

namespace {

constexpr std::string_view kPassSource = "fn main(frag_coord: vec2f) -> vec4f { return helper(); }";
constexpr std::string_view kCommonSource = "fn helper() -> vec4f { return vec4f(1.0); }";

}  // namespace

TEST_CASE("assembled module orders pass, then Common, then generated code", "[shader][assembler]") {
  const std::string code = mewo::assemble_pass_source(kPassSource, kCommonSource);

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
  const std::string code = mewo::assemble_pass_source("// pass comment", "fn helper() {}");

  CHECK(code.find("// pass comment\nfn helper() {}\n") == 0);
}

TEST_CASE("Common source starts on the line after the pass source", "[shader][assembler]") {
  // Step 7's segment map relies on Common starting right after the pass's last line.
  const std::string code = mewo::assemble_pass_source("a\nb\nc", "common");

  CHECK(code.starts_with("a\nb\nc\ncommon\n"));
}

TEST_CASE("empty Common source still assembles", "[shader][assembler]") {
  const std::string code = mewo::assemble_pass_source(kPassSource, "");

  CHECK(code.starts_with(kPassSource));
  CHECK(code.find(mewo::kFragmentEntryPoint) != std::string::npos);
  CHECK(code.find(mewo::kVertexEntryPoint) != std::string::npos);
}
