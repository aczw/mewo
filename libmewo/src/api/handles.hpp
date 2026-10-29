#pragma once

#include "core/runtime.hpp"
#include "core/surface_state.hpp"

#include <memory>
#include <string>

struct mewo_runtime {
  std::unique_ptr<mewo::Runtime> impl;
  std::string current_message;
};

struct mewo_surface {
  std::unique_ptr<mewo::SurfaceState> impl;
};
