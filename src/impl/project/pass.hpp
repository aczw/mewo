#pragma once

#include <cstdint>
#include <string>

namespace mewo::impl {

class Pass {
 public:
  using Id = uint32_t;

  Pass(Id id, std::string name, std::string source);

  Pass(Pass&&) = default;
  Pass& operator=(Pass&&) = default;
  Pass(const Pass&) = delete;
  Pass& operator=(const Pass&) = delete;

  Id id() const { return id_; }

  const std::string& name() const { return name_; }

  const std::string& source() const { return source_; }

 private:
  Id id_;

  std::string name_;
  std::string source_;
};

}  // namespace mewo::impl
