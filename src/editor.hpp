#pragma once

#include "assets.hpp"

#include <string>
#include <string_view>

namespace mewo {

class Editor {
  public:
  Editor(const Assets& assets);

  std::string& visible_code() { return visible_code_; }

  const std::string& visible_code() const { return visible_code_; }

  void set_visible_code(std::string_view visible_code)
  {
    visible_code_ = std::string(visible_code);
  }

  std::string combined_code() const
  {
    // TODO: cache this?
    return prefix_ + "\n\n" + visible_code_;
  }

  private:
  std::string prefix_;
  std::string visible_code_;
};

}
