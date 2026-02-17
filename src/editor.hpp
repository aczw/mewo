#pragma once

#include <string>
#include <string_view>

namespace mewo {

class Editor {
  public:
  Editor(std::string_view initial_code);

  std::string& code();

  private:
  std::string code_;
};

}
