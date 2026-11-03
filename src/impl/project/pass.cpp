#include "impl/project/pass.hpp"

#include <string>
#include <utility>

namespace mewo::impl {

void Pass::set_source(std::string new_source) {
  revision_ += 1;
  source_ = std::move(new_source);
}

}  // namespace mewo::impl
