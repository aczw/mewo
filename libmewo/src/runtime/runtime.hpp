#pragma once

#include "gpu/types.hpp"
#include "runtime/event.hpp"
#include "utility/error.hpp"

#include <deque>
#include <expected>
#include <memory>
#include <optional>
#include <utility>

namespace mewo {

class Runtime {
 public:
  static std::expected<std::unique_ptr<Runtime>, Error> create();

  Runtime(Runtime&&) = delete;
  Runtime& operator=(Runtime&&) = delete;
  Runtime(const Runtime&) = delete;
  Runtime& operator=(const Runtime&) = delete;

  std::optional<Event> poll_event();
  void tick();
  void request_adapter_and_device();

 private:
  explicit Runtime(Instance&& instance) : instance_(std::move(instance)) {}

  void request_device();

  std::deque<Event> event_queue_;

  Instance instance_;
  Adapter adapter_;
  Device device_;
};

}  // namespace mewo
