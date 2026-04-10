#pragma once

#include <TextEditor.h>

namespace mewo {

enum class Theme { DefaultDark, DefaultLight, RosePine };

const TextEditor::Palette& get_palette(Theme theme);

}  // namespace mewo
