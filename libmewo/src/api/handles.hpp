#pragma once

#include "runtime/runtime.hpp"

#include <memory>
#include <string>

struct mewo_runtime {
  std::unique_ptr<mewo::Runtime> impl;
  std::string current_message;
};
