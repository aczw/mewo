#pragma once

#include "core/event.hpp"
#include "core/gpu/types.hpp"
#include "core/pinned.hpp"
#include "impl/runtime/compile_request.hpp"

#include <deque>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace mewo::impl {

class Runtime : private Pinned {
 public:
  static std::optional<std::unique_ptr<Runtime>> create();

  const gpu::Instance& instance() const { return instance_; }

  const gpu::Adapter& adapter() const { return adapter_; }

  const gpu::Device& device() const { return device_; }

  const gpu::Queue& queue() const { return queue_; }

  std::optional<Event> poll_event();
  void tick();
  void request_gfx_components(const gpu::Surface* compatible_surface);

  CompileRequestId start_compile_request(const WGPURenderPipelineDescriptor& render_pipeline_desc);

  std::optional<CompileRequestResult> take_compile_request(CompileRequestId request_id);

 private:
  explicit Runtime(gpu::Instance&& instance)
      : instance_(std::move(instance)), next_compile_request_id_(0) {}

  void request_device_and_queue();

  std::deque<Event> event_queue_;

  gpu::Instance instance_;
  gpu::Adapter adapter_;
  gpu::Device device_;
  gpu::Queue queue_;

  CompileRequestId next_compile_request_id_;
  std::unordered_map<CompileRequestId, CompileRequestResult> finished_compile_requests_;
};

}  // namespace mewo::impl

struct mewo_runtime {
  std::unique_ptr<mewo::impl::Runtime> impl;
  std::string current_message;
};
