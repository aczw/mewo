#pragma once

#include "assets.hpp"

#include <string>
#include <string_view>

namespace mewo {

class Editor {
  public:
  Editor(const Assets& assets);

  std::string& visible_code();
  const std::string& visible_code() const;

  void set_visible_code(std::string_view visible_code);

  std::string combined_code() const;

  private:
  std::string prefix_;
  std::string visible_code_;
};

}
