#include "core/event.hpp"

#include "utility/match.hpp"

#include <string>

namespace mewo {

/*
std::string event_to_string(const Event& event) {
return std::visit(
  Match{
    [](const AdapterRequestFailed& variant) {
      std::string reason = [&variant]() -> std::string {
        switch (variant.reason) {
          case AdapterRequestFailed::Reason::kError: return "of an error";
          case AdapterRequestFailed::Reason::kUnavailable: return "it is unavailable";
        }
        std::unreachable();
      }();

      return std::format(
        "failed to request adapter because {} with the message \"{}\"", reason, variant.message
      );
    },

    [](const DeviceLost& variant) {
      std::string reason = [&variant]() -> std::string {
        switch (variant.reason) {
          case WGPUDeviceLostReason_Unknown: return "of an unknown reason";
          case WGPUDeviceLostReason_Destroyed: return "it was destroyed";
          default: return "of an internal error";
        }
      }();

      return std::format(
        "lost device because {} with the message \"{}\"", reason, variant.message
      );
    },

    [](const GpuErrorOccurred& variant) {
      std::string reason = [&variant]() -> std::string {
        switch (variant.type) {
          case WGPUErrorType_Validation: return "of validation";
          case WGPUErrorType_OutOfMemory: return "it's out of memory";
          case WGPUErrorType_Unknown: return "of an unknown reason";
          default: return "of an internal reason";
        }
      }();

      return std::format(
        "GPU error occurred because {} with the message \"{}\"", reason, variant.message
      );
    },

    [](const DeviceRequestSucceeded&) { return std::string(); },

    [](const DeviceRequestFailed& variant) {
      return std::format("failed to request device with the message \"{}\"", variant.message);
    },
  },
  event
);
}
*/

std::string get_event_message(const Event& event) {
  return std::visit(
    Match{
      [](const AdapterRequestFailed& variant) { return variant.message; },
      [](const DeviceLost& variant) { return variant.message; },
      [](const GpuErrorOccurred& variant) { return variant.message; },
      [](const DeviceRequestSucceeded&) { return std::string(); },
      [](const DeviceRequestFailed& variant) { return variant.message; },
    },
    event
  );
}

}  // namespace mewo
