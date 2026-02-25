#pragma once

#include "assets.hpp"

#include <string>

namespace mewo {

class Editor {
  public:
  Editor(const Assets& assets);

  std::string& visible_code();

  std::string combined_code() const;

  private:
  std::string prefix_;
  std::string visible_code_;
};

}
