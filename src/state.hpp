#pragma once

namespace mewo {

/// Stores general application state. Also lets me avoid passing the entire `Mewo` class
/// into functions, reducing the risk of circular dependencies.
struct State {
  bool should_quit = false;
};

}
