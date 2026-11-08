#pragma once

#include "core/common.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace mewo {

class Pass {
 public:
  using Id = uint32_t;

  Pass(Id id, std::string name, std::string source)
      : id_(id), revision_(0), name_(std::move(name)), source_(std::move(source)) {}

  Pass(Pass&&) = default;
  Pass& operator=(Pass&&) = default;
  Pass(const Pass&) = delete;
  Pass& operator=(const Pass&) = delete;

  Id id() const { return id_; }

  Revision revision() const { return revision_; }

  const std::string& name() const { return name_; }

  const std::string& source() const { return source_; }

  void set_source(std::string new_source);

  /** Does not bump revision. */
  void set_name(std::string new_name);

 private:
  Id id_;
  Revision revision_;

  std::string name_;
  std::string source_;
};

}  // namespace mewo
