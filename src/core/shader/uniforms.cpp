#include "core/shader/uniforms.hpp"

#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace mewo {
namespace {

std::string_view to_string(WgslType type) {
  switch (type) {
    case WgslType::kF32: return "f32";
    case WgslType::kU32: return "u32";
    case WgslType::kVec2f: return "vec2f";
  }
  std::unreachable();
}

}  // namespace

std::string generate_prelude_wgsl_uniforms() {
  std::string source = "struct mw_Uniforms {\n";

  for (size_t idx = 0; idx < kUniformFields.size(); ++idx) {
    const UniformField& field = kUniformFields[idx];
    source += std::format("  {}: {},\n", field.name, to_string(field.type));
  }

  source += "};\n";
  source += "@group(0) @binding(0) var<uniform> mw_uniforms: mw_Uniforms;\n";

  return source;
}

}  // namespace mewo
