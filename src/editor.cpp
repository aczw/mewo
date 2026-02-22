#include "editor.hpp"

#include "exception.hpp"
#include "fs.hpp"
#include "query.hpp"

#include <filesystem>

namespace mewo {

Editor::Editor()
    : prefix_(std::invoke([] -> std::string {
      if constexpr (query::is_release())
        throw Exception("TODO: handle \"frag_prefix.txt\" file path on release");

      std::filesystem::path file_path = "../../assets/shaders/snippets/default_frag_prefix.txt";

      return fs::read_file(file_path);
    }))
    , visible_code_(std::invoke([] -> std::string {
      if constexpr (query::is_release())
        throw Exception("TODO: handle \"default_viewport_frag.txt\" file path on release");

      std::filesystem::path file_path = "../../assets/shaders/snippets/default_viewport_frag.txt";

      return fs::read_file(file_path);
    }))
{
}

std::string& Editor::visible_code() { return visible_code_; }

std::string Editor::combined_code() const { return prefix_ + "\n\n" + visible_code_; }

}
