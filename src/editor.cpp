#include "editor.hpp"

#include "fs.hpp"

namespace mewo {

Editor::Editor(const Assets& assets)
    : prefix_(fs::read_file(assets.get("shaders/snippets/default_frag_prefix.txt")))
    // TODO: when projects are added, it should load its fragment shader and not this default
    , visible_code_(fs::read_wgsl_shader(assets.get("shaders/viewport.frag.wgsl")))
{
}

std::string& Editor::visible_code() { return visible_code_; }

std::string Editor::combined_code() const
{
  // TODO: cache combined code so function isn't allocating a new string
  //       every time it's called?
  return prefix_ + "\n\n" + visible_code_;
}

}
