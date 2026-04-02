#include "queue.hpp"

namespace mewo::event {

std::deque<Event> Queue::drain() {
  std::scoped_lock lock(mutex_);

  std::deque<Event> drained_queue;
  drained_queue.swap(queue_);

  return drained_queue;
}

}  // namespace mewo::event
