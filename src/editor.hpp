#pragma once

#include "gfx/compilation_diagnostic.hpp"

#include <filesystem>
#include <string>
#include <string_view>

namespace mewo {

class Editor {
 public:
  explicit Editor(const std::filesystem::path& assets_dir);

  std::string& visible_code() { return visible_code_; }

  const std::string& visible_code() const { return visible_code_; }

  const gfx::CompilationDiagnostics& diagnostics() const { return diagnostics_; }

  void set_visible_code(std::string_view visible_code) {
    visible_code_ = std::string(visible_code);
  }

  /// Editor always takes ownership of shader compilation diagnostics.
  void set_diagnostics(gfx::CompilationDiagnostics&& diagnostics) {
    diagnostics_ = std::move(diagnostics);
  }

  std::string combined_code() const {
    // TODO: cache this?
    return prefix_ + "\n\n" + visible_code_;
  }

 private:
  std::string prefix_;
  std::string visible_code_;

  gfx::CompilationDiagnostics diagnostics_;
};

}  // namespace mewo
