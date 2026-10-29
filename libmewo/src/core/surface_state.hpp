#pragma once

#include "core/error.hpp"
#include "core/runtime.hpp"
#include "gpu/surface_source.hpp"
#include "gpu/types.hpp"

#include <cstdint>
#include <expected>
#include <memory>

namespace mewo {

class SurfaceState {
 public:
  static std::expected<std::unique_ptr<SurfaceState>, Error> create(
    const Runtime& runtime,
    uint32_t initial_width,
    uint32_t initial_height,
    const SurfaceSource& source
  );

  SurfaceState(SurfaceState&&) = delete;
  SurfaceState& operator=(SurfaceState&&) = delete;
  SurfaceState(const SurfaceState&) = delete;
  SurfaceState& operator=(const SurfaceState&) = delete;

 private:
  explicit SurfaceState(Surface&& surface, uint32_t initial_width, uint32_t initial_height)
      : surface_(std::move(surface)), width_(initial_width), height_(initial_height) {}

  Surface surface_;
  [[maybe_unused]] uint32_t width_;
  [[maybe_unused]] uint32_t height_;
};

}  // namespace mewo
