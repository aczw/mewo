#pragma once

#include "gfx/compilation_diagnostic.hpp"

#include <TextEditor.h>
#include <imgui.h>

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

namespace mewo {

class Editor {
 public:
  explicit Editor(const std::filesystem::path& assets_dir);

  std::string visible_code() const { return impl_.GetText(); }

  const TextEditor::Palette& palette() const { return impl_.GetPalette(); }

  int prefix_line_count() const { return prefix_line_count_; }

  const gfx::CompilationDiagnostics& diagnostics() const { return diagnostics_; }

  void set_visible_code(std::string_view visible_code) { impl_.SetText(visible_code); }

  /// Editor always takes ownership of shader compilation diagnostics.
  void set_diagnostics(gfx::CompilationDiagnostics&& diagnostics) {
    diagnostics_ = std::move(diagnostics);
  }

  void build_layout(const ImVec2& size) { impl_.Render("##editor", size); }

  std::string combined_code() const {
    // TODO: cache this?
    return prefix_ + "\n" + visible_code();
  }

  bool dirty() const { return undo_index_ != impl_.GetUndoIndex(); }

  void save() {
    impl_.StripTrailingWhitespaces();
    undo_index_ = impl_.GetUndoIndex();
  }

 private:
  TextEditor impl_;
  size_t undo_index_ = 0;

  std::string prefix_;
  int prefix_line_count_ = 0;
  gfx::CompilationDiagnostics diagnostics_;
};

}  // namespace mewo
