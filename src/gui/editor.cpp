#include "gui/editor.hpp"

#include "imgui.h"
#include "io.hpp"
#include "query.hpp"

#include <TextEditor.h>

#include <algorithm>
#include <array>
#include <print>
#include <string_view>

namespace mewo {

namespace {

bool is_wgsl_punctuation(ImWchar character) {
  // https://www.w3.org/TR/WGSL/#syntactic-tokens
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

TextEditor::Iterator get_c_style_number(TextEditor::Iterator start, TextEditor::Iterator end) {
  TextEditor::Iterator i = start;
  TextEditor::Iterator marker;

  {
    ImWchar yych;
    unsigned int yyaccept = 0;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '.': goto yy3;
      case '0': goto yy4;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy6;
      default:
        if (i >= end)
          goto yy82;
        goto yy1;
    }
  yy1:
    ++i;
  yy2: { return start; }
  yy3:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy8;
      default: goto yy2;
    }
  yy4:
    yyaccept = 0;
    ++i;
    marker = i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 0x00: goto yy5;
      case 'B':
      case 'b': goto yy16;
      case 'X':
      case 'x': goto yy20;
      default: goto yy13;
    }
  yy5: { return i; }
  yy6:
    yyaccept = 1;
    ++i;
    marker = i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '.': goto yy10;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy6;
      case 'E':
      case 'e': goto yy17;
      case 'L': goto yy22;
      case 'U':
      case 'u': goto yy23;
      case 'l': goto yy24;
      default: goto yy7;
    }
  yy7: { return i; }
  yy8:
    yyaccept = 2;
    ++i;
    marker = i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy8;
      case 'E':
      case 'e': goto yy25;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy26;
      default: goto yy9;
    }
  yy9: { return i; }
  yy10:
    yyaccept = 3;
    ++i;
    marker = i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy8;
      case 'E':
      case 'e': goto yy27;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy28;
      default: goto yy11;
    }
  yy11: { return i; }
  yy12:
    yyaccept = 0;
    ++i;
    marker = i;
    yych = i < end ? *i : 0;
  yy13:
    switch (yych) {
      case '.': goto yy10;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7': goto yy12;
      case '8':
      case '9': goto yy14;
      case 'E':
      case 'e': goto yy17;
      case 'L': goto yy18;
      case 'U':
      case 'u': goto yy19;
      case 'l': goto yy21;
      default: goto yy5;
    }
  yy14:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '.': goto yy10;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy14;
      case 'E':
      case 'e': goto yy17;
      default: goto yy15;
    }
  yy15:
    i = marker;
    switch (yyaccept) {
      case 0: goto yy5;
      case 1: goto yy7;
      case 2: goto yy9;
      case 3: goto yy11;
      default: goto yy40;
    }
  yy16:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1': goto yy29;
      default: goto yy15;
    }
  yy17:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '+':
      case '-': goto yy31;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy32;
      default: goto yy15;
    }
  yy18:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy34;
      case 'U':
      case 'u': goto yy35;
      default: goto yy5;
    }
  yy19:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy36;
      case 'l': goto yy37;
      default: goto yy5;
    }
  yy20:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '.': goto yy38;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f': goto yy39;
      default: goto yy15;
    }
  yy21:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy35;
      case 'l': goto yy34;
      default: goto yy5;
    }
  yy22:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy41;
      case 'U':
      case 'u': goto yy42;
      default: goto yy7;
    }
  yy23:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy43;
      case 'l': goto yy44;
      default: goto yy7;
    }
  yy24:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy42;
      case 'l': goto yy41;
      default: goto yy7;
    }
  yy25:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '+':
      case '-': goto yy45;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy46;
      default: goto yy15;
    }
  yy26:
    ++i;
    goto yy9;
  yy27:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '+':
      case '-': goto yy47;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy48;
      default: goto yy15;
    }
  yy28:
    ++i;
    goto yy11;
  yy29:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1': goto yy29;
      case 'L': goto yy49;
      case 'U':
      case 'u': goto yy50;
      case 'l': goto yy51;
      default: goto yy30;
    }
  yy30: { return i; }
  yy31:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy32;
      default: goto yy15;
    }
  yy32:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy32;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy52;
      default: goto yy33;
    }
  yy33: { return i; }
  yy34:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy35;
      default: goto yy5;
    }
  yy35:
    ++i;
    goto yy5;
  yy36:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy35;
      default: goto yy5;
    }
  yy37:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'l': goto yy35;
      default: goto yy5;
    }
  yy38:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 0x00:
      case 'P':
      case 'p': goto yy15;
      default: goto yy54;
    }
  yy39:
    yyaccept = 4;
    ++i;
    marker = i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '.': goto yy55;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f': goto yy39;
      case 'L': goto yy56;
      case 'P':
      case 'p': goto yy57;
      case 'U':
      case 'u': goto yy58;
      case 'l': goto yy59;
      default: goto yy40;
    }
  yy40: { return i; }
  yy41:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy42;
      default: goto yy7;
    }
  yy42:
    ++i;
    goto yy7;
  yy43:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy42;
      default: goto yy7;
    }
  yy44:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'l': goto yy42;
      default: goto yy7;
    }
  yy45:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy46;
      default: goto yy15;
    }
  yy46:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy46;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy26;
      default: goto yy9;
    }
  yy47:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy48;
      default: goto yy15;
    }
  yy48:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy48;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy28;
      default: goto yy11;
    }
  yy49:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy60;
      case 'U':
      case 'u': goto yy61;
      default: goto yy30;
    }
  yy50:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy62;
      case 'l': goto yy63;
      default: goto yy30;
    }
  yy51:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy61;
      case 'l': goto yy60;
      default: goto yy30;
    }
  yy52:
    ++i;
    goto yy33;
  yy53:
    ++i;
    yych = i < end ? *i : 0;
  yy54:
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case 'A':
      case 'B':
      case 'C':
      case 'D':
      case 'E':
      case 'F':
      case 'a':
      case 'b':
      case 'c':
      case 'd':
      case 'e':
      case 'f': goto yy53;
      case 'P':
      case 'p': goto yy64;
      default: goto yy15;
    }
  yy55:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 0x00: goto yy15;
      case 'P':
      case 'p': goto yy65;
      default: goto yy54;
    }
  yy56:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy66;
      case 'U':
      case 'u': goto yy67;
      default: goto yy40;
    }
  yy57:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '+':
      case '-': goto yy68;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy69;
      default: goto yy15;
    }
  yy58:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy71;
      case 'l': goto yy72;
      default: goto yy40;
    }
  yy59:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy67;
      case 'l': goto yy66;
      default: goto yy40;
    }
  yy60:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy61;
      default: goto yy30;
    }
  yy61:
    ++i;
    goto yy30;
  yy62:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy61;
      default: goto yy30;
    }
  yy63:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'l': goto yy61;
      default: goto yy30;
    }
  yy64:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '+':
      case '-': goto yy73;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy74;
      default: goto yy15;
    }
  yy65:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '+':
      case '-': goto yy76;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy77;
      default: goto yy15;
    }
  yy66:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'U':
      case 'u': goto yy67;
      default: goto yy40;
    }
  yy67:
    ++i;
    goto yy40;
  yy68:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy69;
      default: goto yy15;
    }
  yy69:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy69;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy79;
      default: goto yy70;
    }
  yy70: { return i; }
  yy71:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'L': goto yy67;
      default: goto yy40;
    }
  yy72:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case 'l': goto yy67;
      default: goto yy40;
    }
  yy73:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy74;
      default: goto yy15;
    }
  yy74:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy74;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy80;
      default: goto yy75;
    }
  yy75: { return i; }
  yy76:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy77;
      default: goto yy15;
    }
  yy77:
    ++i;
    yych = i < end ? *i : 0;
    switch (yych) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': goto yy77;
      case 'F':
      case 'L':
      case 'f':
      case 'l': goto yy81;
      default: goto yy78;
    }
  yy78: { return i; }
  yy79:
    ++i;
    goto yy70;
  yy80:
    ++i;
    goto yy75;
  yy81:
    ++i;
    goto yy78;
  yy82: { return start; }
  }
}

const TextEditor::Language* get_wgsl_language() {
  static TextEditor::Language language = {
    .name = "WGSL",
    .singleLineComment = "//",
    .commentStart = "/*",
    .commentEnd = "*/",
    .isPunctuation = is_wgsl_punctuation,
    .getIdentifier =
      [](TextEditor::Iterator start, TextEditor::Iterator end) {
        if (start < end && TextEditor::CodePoint::isXidStart(*start)) {
          start++;

          while (start < end && TextEditor::CodePoint::isXidContinue(*start)) {
            start++;
          }
        }

        return start;
      },
    .getNumber = get_c_style_number,
  };

  // https://www.w3.org/TR/WGSL/#keyword-summary
  static constexpr auto WGSL_KEYWORDS = std::to_array<std::string_view>({
    "alias",   "break",      "case",    "const", "const_assert", "continue", "continuing",
    "default", "diagnostic", "discard", "else",  "enable",       "false",    "fn",
    "for",     "if",         "let",     "loop",  "override",     "requires", "return",
    "struct",  "switch",     "true",    "var",   "while",
  });

  for (const auto& keyword : WGSL_KEYWORDS)
    language.keywords.insert(keyword.data());

  return &language;
}

}  // namespace

Editor::Editor(const std::filesystem::path& assets_dir)
    : undo_index_(impl_.GetUndoIndex()),
      prefix_(io::read_file(assets_dir / "shaders/snippets/default_frag_prefix.txt")),
      // There is no newline at the end of the file so we manually increment by 1
      prefix_line_count_(static_cast<int>(std::count(prefix_.begin(), prefix_.end(), '\n')) + 1) {
  impl_.SetShowWhitespacesEnabled(false);
  impl_.SetShowScrollbarMiniMapEnabled(false);
  impl_.SetLanguage(get_wgsl_language());

  if constexpr (query::is_debug())
    std::println("Editor language: {}", impl_.GetLanguageName());

  set_visible_code(io::read_wgsl_shader(assets_dir / "shaders/viewport.frag.wgsl"));
}

}  // namespace mewo
