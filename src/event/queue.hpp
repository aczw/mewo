#pragma once

#include "event.hpp"

#include <deque>
#include <mutex>

namespace mewo::event {

class Queue {
 public:
  void enqueue(Event event) {
    std::scoped_lock lock(mutex_);
    queue_.push_back(std::move(event));
  }

  std::deque<Event> drain();

 private:
  mutable std::mutex mutex_;
  std::deque<Event> queue_;
};

}  // namespace mewo::event
