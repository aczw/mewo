#include "editor.hpp"

namespace mewo {

Editor::Editor(std::string_view code)
    : code_(code)
{
}

std::string& Editor::code() { return code_; }

}
