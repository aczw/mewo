#pragma once

#include "core/pinned.hpp"
#include "impl/project/pass.hpp"

#include <memory>
#include <vector>

namespace mewo::impl {

class Project : private Pinned {
 public:
  static std::unique_ptr<Project> create();

  const std::vector<Pass>& passes() const { return passes_; }

 private:
  explicit Project(std::vector<Pass>&& passes) : passes_(std::move(passes)) {}

  std::vector<Pass> passes_;
};

}  // namespace mewo::impl

struct mewo_project {
  std::unique_ptr<mewo::impl::Project> impl;
};
