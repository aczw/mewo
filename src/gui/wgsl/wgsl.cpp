#include "wgsl.hpp"

#include "get_wgsl_number.hpp"

namespace mewo::language {

namespace {

/// See // From https://www.w3.org/TR/WGSL/#syntactic-tokens.
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

}  // namespace

const TextEditor::Language* wgsl() {
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

    for (const auto& keyword : WGSL_KEYWORDS)
      language.keywords.insert(keyword.data());

    initialized = true;
  }

  return &language;
}

}  // namespace mewo::language
