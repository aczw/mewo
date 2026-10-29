#pragma once

#include "core/error.hpp"
#include "gpu/surface_source.hpp"
#include "gpu/types.hpp"

#include <cstdint>
#include <expected>
#include <memory>

namespace mewo {

class SurfaceState {
 public:
  static std::expected<std::unique_ptr<SurfaceState>, Error> create(
    const Instance& instance,
    uint32_t initial_width,
    uint32_t initial_height,
    const SurfaceSource& source
  );

  SurfaceState(SurfaceState&&) = delete;
  SurfaceState& operator=(SurfaceState&&) = delete;
  SurfaceState(const SurfaceState&) = delete;
  SurfaceState& operator=(const SurfaceState&) = delete;

  void resize(uint32_t new_width, uint32_t new_height);

  const Surface& surface() const { return surface_; }

 private:
  explicit SurfaceState(Surface&& surface, uint32_t initial_width, uint32_t initial_height)
      : surface_(std::move(surface)),
        width_(initial_width),
        height_(initial_height),
        needs_configure_(false) {}

  Surface surface_;
  uint32_t width_;
  uint32_t height_;
  bool needs_configure_;
};

}  // namespace mewo
