#pragma once

#include "runtime/runtime.hpp"

#include <memory>

struct mewo_runtime {
  std::unique_ptr<mewo::Runtime> impl;
};
