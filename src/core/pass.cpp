#include "core/pass.hpp"

#include <string>
#include <utility>

namespace mewo {

void Pass::set_source(std::string new_source) {
  revision_ += 1;
  source_ = std::move(new_source);
}

}  // namespace mewo
