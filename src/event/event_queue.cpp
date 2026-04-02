#include "event_queue.hpp"

namespace mewo {

std::deque<Event> EventQueue::drain() {
  std::scoped_lock lock(mutex_);

  std::deque<Event> drained_queue;
  drained_queue.swap(queue_);

  return drained_queue;
}

}  // namespace mewo
