#pragma once

#include "core/common.hpp"
#include "core/pass.hpp"
#include "core/pinned.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace mewo::impl {

class Project : private Pinned {
 public:
  enum class RemovePassResult {
    kSuccess,
    kNotFound,
    /** Returned when there's only one pass left in the project. */
    kFailedBecauseLast,
  };

  static std::unique_ptr<Project> create();

  const std::vector<Pass>& passes() const { return passes_; }

  const std::string& common_source() const { return common_source_; }

  Revision common_revision() const { return common_revision_; }

  Revision structure_revision() const { return structure_revision_; }

  Pass::Id add_pass(std::string name, std::string source);

  RemovePassResult remove_pass(Pass::Id pass_id);

  /** @return True if the move occurred (even no-op), false if the pass ID or index is invalid. */
  bool move_pass(Pass::Id pass_id, uint32_t new_index);

  /**
   * Replaces the pass' source and bumps its revision, even if the text has not changed.
   *
   * @return True if the operation succeeded, false if the pass with the given ID was not found.
   */
  bool set_pass_source(Pass::Id pass_id, std::string new_source);

  void set_common_source(std::string new_source);

 private:
  Project() = default;

  /**
   * Single source of truth for the order in which passes operate.
   */
  std::vector<Pass> passes_;
  Pass::Id next_pass_id_ = 1;

  std::string common_source_;
  Revision common_revision_ = 0;

  Revision structure_revision_ = 0;
};

}  // namespace mewo::impl

struct mewo_project {
  std::unique_ptr<mewo::impl::Project> impl;
};
