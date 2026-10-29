#pragma once

#include "core/error.hpp"
#include "core/event.hpp"
#include "gpu/types.hpp"

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

  const Instance& instance() const { return instance_; }

  std::optional<Event> poll_event();
  void tick();
  void request_adapter_and_device(const Surface& surface);

 private:
  explicit Runtime(Instance&& instance) : instance_(std::move(instance)), adapter_(), device_() {}

  void request_device();

  std::deque<Event> event_queue_;

  Instance instance_;
  Adapter adapter_;
  Device device_;
};

}  // namespace mewo
