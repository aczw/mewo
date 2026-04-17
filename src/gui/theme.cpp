#include "theme.hpp"

#include "util/enum_unreachable.hpp"

namespace mewo {

namespace {

/// See https://rosepinetheme.com/palette.
const TextEditor::Palette& get_rose_pine_palette() {
  const static TextEditor::Palette PALETTE = {{
    IM_COL32(224, 222, 244, 255),  // text
    IM_COL32(49, 116, 143, 255),   // keyword
    IM_COL32(90, 0, 0, 255),       // declaration
    IM_COL32(235, 188, 186, 255),  // number
    IM_COL32(246, 193, 119, 255),  // string
    IM_COL32(144, 140, 170, 255),  // punctuation
    IM_COL32(49, 116, 143, 255),   // preprocessor
    IM_COL32(224, 222, 244, 255),  // identifier
    IM_COL32(235, 188, 186, 255),  // known identifier
    IM_COL32(110, 106, 134, 255),  // comment
    IM_COL32(25, 23, 36, 255),     // background
    IM_COL32(144, 140, 170, 255),  // cursor
    IM_COL32(31, 29, 46, 255),     // selection
    IM_COL32(110, 106, 134, 255),  // whitespace
    IM_COL32(64, 61, 82, 255),     // matchingBracketBackground
    IM_COL32(144, 140, 170, 255),  // matchingBracketActive
    IM_COL32(144, 140, 170, 255),  // matchingBracketLevel1
    IM_COL32(144, 140, 170, 255),  // matchingBracketLevel2
    IM_COL32(144, 140, 170, 255),  // matchingBracketLevel3
    IM_COL32(235, 111, 146, 255),  // matchingBracketError
    IM_COL32(144, 140, 170, 255),  // line number
    IM_COL32(224, 222, 244, 255),  // current line number
  }};

  return PALETTE;
}

}  // namespace

const TextEditor::Palette& get_palette(Theme theme) {
  switch (theme) {
    case Theme::DefaultDark: return TextEditor::GetDarkPalette();
    case Theme::DefaultLight: return TextEditor::GetLightPalette();
    case Theme::RosePine: return get_rose_pine_palette();

    default: util::enum_unreachable("Editor::Theme", theme);
  }
}

}  // namespace mewo
