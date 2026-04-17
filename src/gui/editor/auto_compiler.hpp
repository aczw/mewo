#pragma once

#include "event/event_queue.hpp"

#include <cassert>
#include <cstdint>
#include <optional>

namespace mewo {

class AutoCompiler {
 public:
  bool is_running() const { return elapsed_.has_value(); }

  void start() {
    assert(!elapsed_.has_value());
    elapsed_ = 0;
  }

  void reset() {
    assert(elapsed_.has_value());
    elapsed_ = 0;
  }

  void update(EventQueue& event_queue, uint64_t delta_time);

 private:
  std::optional<uint64_t> elapsed_;  ///< In milliseconds.
};

}  // namespace mewo
