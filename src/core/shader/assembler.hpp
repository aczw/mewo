#pragma once

#include <string>
#include <string_view>

namespace mewo {

inline constexpr std::string_view kVertexEntryPoint = "mw_vs_main";
inline constexpr std::string_view kFragmentEntryPoint = "mw_fs_main";

std::string assemble_pass_source(std::string_view pass_source);

}  // namespace mewo
