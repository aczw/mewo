#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace mewo {

/// Collects pending operations to be applied and processed in the next frame.
class Pending {
 public:
  std::optional<std::string>& run() { return run_; }

  void request_run(std::string_view new_combined_code) { run_ = std::string(new_combined_code); }

 private:
  /// Stores combined fragment shader.
  std::optional<std::string> run_;
};

}  // namespace mewo
