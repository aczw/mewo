#pragma once

#include "gpu/types.hpp"
#include "utility/error.hpp"

#include <expected>
#include <utility>

namespace mewo {

class Runtime {
 public:
  static std::expected<Runtime, Error> create();

  Runtime(Runtime&&) noexcept = default;
  Runtime& operator=(Runtime&&) noexcept = default;
  Runtime(const Runtime&) = delete;
  Runtime& operator=(const Runtime&) = delete;

 private:
  explicit Runtime(Instance instance) : instance_(std::move(instance)) {}

  Instance instance_;
};

}  // namespace mewo
