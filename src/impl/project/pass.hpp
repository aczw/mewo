#pragma once

#include <string>

namespace mewo::impl {

class Pass {
 public:
  Pass(std::string name);

  Pass(Pass&&) = default;
  Pass& operator=(Pass&&) = default;
  Pass(const Pass&) = delete;
  Pass& operator=(const Pass&) = delete;

  const std::string& name() const { return name_; }

 private:
  std::string name_;
};

}  // namespace mewo::impl
