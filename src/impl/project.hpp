#pragma once

#include "core/common.hpp"
#include "core/pass.hpp"
#include "core/pinned.hpp"
#include "core/validation.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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

  /** @return Nothing if the project name or the first pass name breaks the name rules. */
  static std::optional<std::unique_ptr<Project>> create(
    std::string_view project_name,
    std::string&& first_pass_name,
    std::string&& first_pass_source
  );

  const std::string& name() const { return name_; }

  bool set_name(std::string_view new_name);

  const std::vector<Pass>& passes() const { return passes_; }

  const std::string& common_source() const { return common_source_; }

  Revision common_revision() const { return common_revision_; }

  Revision structure_revision() const { return structure_revision_; }

  /**
   * Checks whether `name` could be used for a pass. We follow the name rules and check
   * no other pass has the same name, compared case-insensitively.
   *
   * @param except The pass being renamed, whose own name doesn't count as taken. Nothing when
   * checking a name for a new pass.
   * @return Nothing if the name can be used, otherwise the first error found.
   */
  std::optional<NameError> check_pass_name(
    std::string_view name,
    std::optional<Pass::Id> except = std::nullopt
  ) const;

  /**
   * @return The new pass' ID, or nothing if `check_pass_name` rejects the name.
   */
  std::optional<Pass::Id> add_pass(std::string name, std::string source);

  RemovePassResult remove_pass(Pass::Id pass_id);

  /** @return True if the move occurred (even no-op), false if the pass ID or index is invalid. */
  bool move_pass(Pass::Id pass_id, uint32_t new_index);

  /**
   * Replaces the pass' source and bumps its revision, even if the text has not changed.
   *
   * @return True if the operation succeeded, false if the pass with the given ID was not found.
   */
  bool set_pass_source(Pass::Id pass_id, std::string new_source);

  /**
   * Renames the pass. Bumps no revision, since sessions don't depend on pass names. A pass can be
   * renamed to a different capitalization of its own name.
   *
   * @return True if the operation succeeded, false if the pass with the given ID was not found or
   * `check_pass_name` rejects the name.
   */
  bool set_pass_name(Pass::Id pass_id, std::string new_name);

  void set_common_source(std::string new_source);

 private:
  Project() = default;

  Pass* find_pass(Pass::Id pass_id);

  /** True if a pass other than `except` has this name, compared case-insensitively. */
  bool is_name_taken(std::string_view name, std::optional<Pass::Id> except = std::nullopt) const;

  std::string name_;

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
