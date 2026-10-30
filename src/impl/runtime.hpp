#pragma once

#include "core/event.hpp"
#include "core/gpu/types.hpp"

#include <deque>
#include <memory>
#include <optional>
#include <utility>

namespace mewo::impl {

class Runtime {
 public:
  static std::optional<std::unique_ptr<Runtime>> create();

  Runtime(Runtime&&) = delete;
  Runtime& operator=(Runtime&&) = delete;
  Runtime(const Runtime&) = delete;
  Runtime& operator=(const Runtime&) = delete;

  const gpu::Instance& instance() const { return instance_; }

  std::optional<Event> poll_event();
  void tick();
  void request_adapter_and_device(const gpu::Surface& surface);

 private:
  explicit Runtime(gpu::Instance&& instance)
      : instance_(std::move(instance)), adapter_(), device_() {}

  void request_device();

  std::deque<Event> event_queue_;

  gpu::Instance instance_;
  gpu::Adapter adapter_;
  gpu::Device device_;
};

}  // namespace mewo::impl

struct mewo_runtime {
  std::unique_ptr<mewo::impl::Runtime> impl;
  std::string current_message;
};
