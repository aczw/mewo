#pragma once

#include "core/pinned.hpp"
#include "impl/project/pass.hpp"

#include <memory>
#include <string>
#include <vector>

namespace mewo::impl {

class Project : private Pinned {
 public:
  static std::unique_ptr<Project> create();

  const std::vector<Pass>& passes() const { return passes_; }

  void add_pass(std::string name, std::string source);

  /**
   * Replaces the pass' source and bumps its revision, even if the text has not changed.
   *
   * @return True if the operation succeeded, false if the pass with the given ID was not found.
   */
  bool set_pass_source(Pass::Id pass_id, std::string new_source);

 private:
  Project() = default;

  /**
   * Single source of truth for the order in which passes operate.
   */
  std::vector<Pass> passes_;
  Pass::Id next_pass_id_ = 1;
};

}  // namespace mewo::impl

struct mewo_project {
  std::unique_ptr<mewo::impl::Project> impl;
};
