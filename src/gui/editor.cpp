#include "gui/editor.hpp"

#include "io.hpp"

namespace mewo {

Editor::Editor(const std::filesystem::path& assets_dir)
    : prefix_(io::read_file(assets_dir / "shaders/snippets/default_frag_prefix.txt")),
      visible_code_(io::read_wgsl_shader(assets_dir / "shaders/viewport.frag.wgsl")) {}

}  // namespace mewo
