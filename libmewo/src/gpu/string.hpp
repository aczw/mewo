#pragma once

#include <webgpu/webgpu.h>

#include <optional>
#include <string>

namespace mewo {

/**
 * String views returned by WebGPU are often allocated and freed in a very specific time frame,
 * so this helps convert one to a `std::string` to guarantee it will exist.
 *
 * @return Newly allocated string containing the WebGPU string view data.
 */
std::optional<std::string> to_string(WGPUStringView);

}  // namespace mewo
