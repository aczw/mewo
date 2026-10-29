#pragma once

#include "core/event.hpp"
#include "core/gpu_types.hpp"

#include <deque>
#include <memory>
#include <optional>
#include <utility>

namespace mewo {

class RuntimeImpl {
 public:
  static std::optional<std::unique_ptr<RuntimeImpl>> create();

  RuntimeImpl(RuntimeImpl&&) = delete;
  RuntimeImpl& operator=(RuntimeImpl&&) = delete;
  RuntimeImpl(const RuntimeImpl&) = delete;
  RuntimeImpl& operator=(const RuntimeImpl&) = delete;

  const Instance& instance() const { return instance_; }

  std::optional<Event> poll_event();
  void tick();
  void request_adapter_and_device(const Surface& surface);

 private:
  explicit RuntimeImpl(Instance&& instance)
      : instance_(std::move(instance)), adapter_(), device_() {}

  void request_device();

  std::deque<Event> event_queue_;

  Instance instance_;
  Adapter adapter_;
  Device device_;
};

}  // namespace mewo
