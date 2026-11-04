#pragma once

#include "core/compilation/line.hpp"

#include <optional>
#include <string>
#include <string_view>

namespace mewo {

inline constexpr std::string_view kVertexEntryPoint = "mw_vs_main";
inline constexpr std::string_view kFragmentEntryPoint = "mw_fs_main";

/**
 * Segments contain at least 1 line.
 */
class SegmentMap {
 public:
  /**
   * `start` and `end` are inclusive.
   */
  struct Range {
    LineNum start;
    LineNum end;
  };

  SegmentMap(Range pass, Range common, Range prelude);

  SegmentMap(SegmentMap&&) = default;
  SegmentMap& operator=(SegmentMap&&) = default;
  SegmentMap(const SegmentMap&) = default;
  SegmentMap& operator=(const SegmentMap&) = default;

  /**
   * Line numbers start at 1. If 0 or an out-of-bounds line number is passed,
   * we return `std::nullopt`.
   */
  std::optional<RelativeLine> find_relative_line(LineNum line_number) const;

 private:
  Range pass_;
  Range common_;
  Range prelude_;
};

struct AssembledSource {
  std::string code;
  SegmentMap segments;
};

AssembledSource assemble_pass_source(std::string_view pass_source, std::string_view common_source);

}  // namespace mewo
