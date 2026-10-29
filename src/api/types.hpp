#pragma once

#include "runtime_impl.hpp"
#include "surface_impl.hpp"

#include <memory>
#include <string>

struct mewo_runtime {
  std::unique_ptr<mewo::RuntimeImpl> impl;
  std::string current_message;
};

struct mewo_surface {
  std::unique_ptr<mewo::SurfaceImpl> impl;
};
