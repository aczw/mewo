#include "runtime/runtime.hpp"

#include "gpu/types.hpp"
#include "utility/error.hpp"

#include <webgpu/webgpu.h>

#include <expected>

namespace mewo {

std::expected<Runtime, Error> Runtime::create() {
  WGPUInstance instance = wgpuCreateInstance(nullptr);

  if (!instance)
    return std::unexpected(Error::kInstanceCreationFailed);

  return Runtime(Instance(instance));
}

}  // namespace mewo
