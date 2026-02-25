#include "editor.hpp"

#include "fs.hpp"

namespace mewo {

Editor::Editor(const Assets& assets)
    : prefix_(fs::read_file(assets.get("shaders/snippets/default_frag_prefix.txt")))
    , visible_code_(fs::read_file(assets.get("shaders/snippets/default_viewport_frag.txt")))
{
}

std::string& Editor::visible_code() { return visible_code_; }

std::string Editor::combined_code() const { return prefix_ + "\n\n" + visible_code_; }

}
