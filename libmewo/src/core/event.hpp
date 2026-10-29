#pragma once

#include <webgpu/webgpu.h>

#include <string>
#include <variant>

namespace mewo {

struct AdapterRequestFailed {
  enum class Reason { kError, kUnavailable };

  Reason reason;
  std::string message;
};

struct DeviceLost {
  WGPUDeviceLostReason reason;
  std::string message;
};

struct GpuErrorOccurred {
  WGPUErrorType type;
  std::string message;
};

struct DeviceRequestSucceeded {};

struct DeviceRequestFailed {
  std::string message;
};

using Event = std::variant<
  AdapterRequestFailed,
  DeviceLost,
  GpuErrorOccurred,
  DeviceRequestSucceeded,
  DeviceRequestFailed
>;

std::string get_event_message(const Event& event);

}  // namespace mewo
