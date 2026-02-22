#pragma once

#include <string>

namespace mewo {

class Editor {
  public:
  Editor();

  std::string& visible_code();

  std::string combined_code() const;

  private:
  std::string prefix_;
  std::string visible_code_;
};

}
