#include "editor.hpp"

#include "io.hpp"

namespace mewo {

Editor::Editor(const Assets& assets)
    : prefix_(io::read_file(assets.get("shaders/snippets/default_frag_prefix.txt"))),
      // TODO: when projects are added, it should load its fragment shader and not this default
      visible_code_(io::read_wgsl_shader(assets.get("shaders/viewport.frag.wgsl"))) {}

}  // namespace mewo
