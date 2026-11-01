#pragma once

namespace mewo {

class Pinned {
 public:
  Pinned(Pinned&&) = delete;
  Pinned& operator=(Pinned&&) = delete;
  Pinned(const Pinned&) = delete;
  Pinned& operator=(const Pinned&) = delete;

 protected:
  /**
   * No one can create a bare `Pinned` instance.
   */
  Pinned() = default;

  /**
   * Nobody can `delete` through a `Pinned*`.
   */
  ~Pinned() = default;
};

}  // namespace mewo
