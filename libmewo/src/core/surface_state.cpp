#include "core/surface_state.hpp"

#include "gpu/surface_source.hpp"
#include "gpu/types.hpp"

#include <expected>
#include <memory>
#include <utility>

namespace mewo {

std::expected<std::unique_ptr<SurfaceState>, Error> SurfaceState::create(
  const Runtime& runtime,
  uint32_t initial_width,
  uint32_t initial_height,
  const SurfaceSource& source
) {
  Surface surface = create_surface_from_source(runtime.instance(), source);

  if (!surface)
    return std::unexpected(Error::kSurfaceCreationFailed);

  return std::unique_ptr<SurfaceState>(
    new SurfaceState(std::move(surface), initial_width, initial_height)
  );
}

}  // namespace mewo
