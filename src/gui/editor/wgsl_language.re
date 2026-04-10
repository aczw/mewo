#include "wgsl_language.hpp"

#include <array>
#include <string_view>

namespace mewo {

namespace {

/// See https://www.w3.org/TR/WGSL/#syntactic-tokens.
bool is_wgsl_punctuation(ImWchar character) {
  static constexpr auto ASCII_LUT = std::to_array({
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, true,  false, false, false, true,  true,
    false, true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false,
    false, false, false, false, false, false, true,  true,  true,  true,  true,  false, true,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    true,  false, true,  true,  true,  false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, false, false, false, false, false, false, false,
    false, false, false, false, false, false, true,  true,  true,  true,  false,
  });

  return character < 127 ? ASCII_LUT[character] : false;
}

/// Note that this is the exact same as C-style identifiers. This is technically ignoring
/// some exceptions in the standard (identifiers cannot simply be "_" or begin with "__").
///
/// See https://www.w3.org/TR/WGSL/#identifiers.
TextEditor::Iterator get_wgsl_identifier(TextEditor::Iterator start, TextEditor::Iterator end) {
  if (start < end && TextEditor::CodePoint::isXidStart(*start)) {
    start++;

    while (start < end && TextEditor::CodePoint::isXidContinue(*start))
      start++;
  }

  return start;
}

/// Auto-generated via re2c. Use https://re2c.org/playground, options: "-i -W -Werror"
TextEditor::Iterator get_wgsl_number(TextEditor::Iterator start, TextEditor::Iterator end) {
  TextEditor::Iterator i = start;
  TextEditor::Iterator marker;

  /*!re2c
    re2c:api = custom;
    re2c:api:style = free-form;
    re2c:define:YYCTYPE = ImWchar;
    re2c:define:YYPEEK = "i < end ? *i : 0";
    re2c:define:YYSKIP = "++i;";
    re2c:define:YYBACKUP = "marker = i;";
    re2c:define:YYRESTORE = "i = marker;";
    re2c:define:YYLESSTHAN = "i >= end";
    re2c:yyfill:enable = 0;

    digit      = [0-9];
    hexdigit   = [0-9a-fA-F];
    dec_exp    = [eE] [+-]? digit+;
    hex_exp    = [pP] [+-]? digit+;

    dec_int    = "0" | [1-9] digit*;
    dec_float  = digit+ "." digit* dec_exp? [fh]?
                 | "." digit+ dec_exp? [fh]?
                 | digit+ dec_exp [fh]?
                 | digit+ [fh];

    hex_prefix = "0" [xX];
    hex_int    = hex_prefix hexdigit+ [iu]?;
    hex_float  = hex_prefix (
                     hexdigit+ "." hexdigit* hex_exp? [fh]?
                     | "." hexdigit+ hex_exp? [fh]?
                     | hexdigit+ hex_exp [fh]?
                 );

    wgsl_number = hex_float | hex_int | dec_float | dec_int [iu]?;

    wgsl_number { return i; }
    *           { return start; }
  */
}

}  // namespace

const TextEditor::Language* get_wgsl_language() {
  static bool initialized = false;
  static TextEditor::Language language;

  if (!initialized) {
    language = {
      .name = "WGSL",
      .singleLineComment = "//",
      .commentStart = "/*",
      .commentEnd = "*/",
      .isPunctuation = is_wgsl_punctuation,
      .getIdentifier = get_wgsl_identifier,
      .getNumber = get_wgsl_number,
    };

    // From https://www.w3.org/TR/WGSL/#keyword-summary
    static constexpr auto WGSL_KEYWORDS = std::to_array<std::string_view>({
      "alias",   "break",      "case",    "const", "const_assert", "continue", "continuing",
      "default", "diagnostic", "discard", "else",  "enable",       "false",    "fn",
      "for",     "if",         "let",     "loop",  "override",     "requires", "return",
      "struct",  "switch",     "true",    "var",   "while",
    });

    // Treat predeclared types as keywords. See https://www.w3.org/TR/WGSL/#predeclared-types
    static constexpr auto WGSL_TYPES = std::to_array<std::string_view>({
      "bool",
      "f16",
      "f32",
      "i32",
      "sampler",
      "sampler_comparison",
      "texture_depth_2d",
      "texture_depth_2d_array",
      "texture_depth_cube",
      "texture_depth_cube_array",
      "texture_depth_multisampled_2d",
      "texture_external",
      "u32",
      "array",
      "atomic",
      "mat2x2",
      "mat2x2f",
      "mat2x2h",
      "mat2x3",
      "mat2x3f",
      "mat2x3h",
      "mat2x4",
      "mat2x4f",
      "mat2x4h",
      "mat3x2",
      "mat3x2f",
      "mat3x2h",
      "mat3x3",
      "mat3x3f",
      "mat3x3h",
      "mat3x4",
      "mat3x4f",
      "mat3x4h",
      "mat4x2",
      "mat4x2f",
      "mat4x2h",
      "mat4x3",
      "mat4x3f",
      "mat4x3h",
      "mat4x4",
      "mat4x4f",
      "mat4x4h",
      "ptr",
      "texture_1d",
      "texture_2d",
      "texture_2d_array",
      "texture_3d",
      "texture_cube",
      "texture_cube_array",
      "texture_multisampled_2d",
      "texture_storage_1d",
      "texture_storage_2d",
      "texture_storage_2d_array",
      "texture_storage_3d",
      "vec2",
      "vec2i",
      "vec2u",
      "vec2f",
      "vec2h",
      "vec3",
      "vec3i",
      "vec3u",
      "vec3f",
      "vec3h",
      "vec4",
      "vec4i",
      "vec4u",
      "vec4f",
      "vec4h",
    });

    for (const auto& keyword : WGSL_KEYWORDS)
      language.keywords.insert(keyword.data());
    for (const auto& type : WGSL_TYPES)
      language.keywords.insert(type.data());

    initialized = true;
  }

  return &language;
}

}  // namespace mewo
