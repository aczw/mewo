#pragma once

#include "event/event_queue.hpp"
#include "gfx/compilation_diagnostic.hpp"
#include "gui/theme.hpp"

#include <TextEditor.h>
#include <imgui.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

namespace mewo {

class Editor {
 public:
  Editor(EventQueue& event_queue, const std::filesystem::path& assets_dir, Theme theme);

  const TextEditor& impl() const { return impl_; }

  std::string visible_code() const { return impl_.GetText(); }

  int prefix_line_count() const { return prefix_line_count_; }

  const gfx::CompilationDiagnostics& diagnostics() const { return diagnostics_; }

  void set_visible_code(std::string_view visible_code) { impl_.SetText(visible_code); }

  /// Editor always takes ownership of shader compilation diagnostics.
  void set_diagnostics(gfx::CompilationDiagnostics&& diagnostics) {
    diagnostics_ = std::move(diagnostics);
  }

  void build_layout(const ImVec2& size) { impl_.Render("##Editor", size); }

  std::string combined_code() const {
    // TODO: cache this?
    return prefix_ + "\n" + visible_code();
  }

  void save() {
    impl_.StripTrailingWhitespaces();
    undo_index_ = impl_.GetUndoIndex();
  }

  void update_theme(Theme theme) { impl_.SetPalette(get_palette(theme)); }

  bool is_dirty() const { return impl_.GetUndoIndex() != undo_index_; }

 private:
  TextEditor impl_;

  size_t undo_index_ = 0;

  std::string prefix_;
  int prefix_line_count_ = 0;
  gfx::CompilationDiagnostics diagnostics_;
};

}  // namespace mewo
