#include "gui/editor/editor.hpp"

#include "gui/editor/wgsl_language.hpp"
#include "io.hpp"

#include <TextEditor.h>

#include <algorithm>

namespace mewo {

Editor::Editor(const std::filesystem::path& assets_dir)
    : undo_index_(impl_.GetUndoIndex()),
      prefix_(io::read_file(assets_dir / "shaders/snippets/default_frag_prefix.txt")),
      // There is no newline at the end of the file so we manually increment by 1
      prefix_line_count_(static_cast<int>(std::count(prefix_.begin(), prefix_.end(), '\n')) + 1) {
  impl_.SetShowWhitespacesEnabled(false);
  impl_.SetShowScrollbarMiniMapEnabled(false);
  impl_.SetLanguage(get_wgsl_language());

  set_visible_code(io::read_wgsl_shader(assets_dir / "shaders/viewport.frag.wgsl"));
}

}  // namespace mewo
