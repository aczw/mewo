#pragma once

namespace mewo::sdl {

/// Initializes and maintains SDL. Copying or moving this object does not make sense.
class Context {
  public:
  Context();
  ~Context();

  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;
};

}
