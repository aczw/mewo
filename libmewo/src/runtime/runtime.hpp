#pragma once

#include "gpu/types.hpp"
#include "utility/error.hpp"
#include "utility/event.hpp"

#include <deque>
#include <expected>
#include <memory>
#include <utility>

namespace mewo {

class Runtime {
 public:
  static std::expected<std::unique_ptr<Runtime>, Error> create();

  Runtime(Runtime&&) = delete;
  Runtime& operator=(Runtime&&) = delete;
  Runtime(const Runtime&) = delete;
  Runtime& operator=(const Runtime&) = delete;

  /**
   * Requests the WebGPU adapter and device.
   */
  void request_device();

 private:
  explicit Runtime(Instance&& instance) : instance_(std::move(instance)) {}

  std::deque<Event> event_queue_;

  Instance instance_;
  Adapter adapter_;
};

}  // namespace mewo
