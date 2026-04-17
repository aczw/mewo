#include "auto_compiler.hpp"

#include "event/event.hpp"

namespace mewo {

void AutoCompiler::update(EventQueue& event_queue, uint64_t delta_time) {
  if (!elapsed_)
    return;

  uint64_t elapsed = elapsed_.value() + delta_time;

  if (elapsed >= 300) {
    event_queue.push(AutoCompilerElapsed{});
    elapsed_.reset();
  } else {
    elapsed_ = elapsed;
  }
}

}  // namespace mewo
