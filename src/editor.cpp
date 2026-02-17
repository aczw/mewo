#include "editor.hpp"

namespace mewo {

Editor::Editor(std::string_view initial_code)
    : code_(initial_code)
{
}

std::string& Editor::code() { return code_; }

}
