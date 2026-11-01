#include "impl/project/pass.hpp"

#include <string>
#include <utility>

namespace mewo::impl {

Pass::Pass(Id id, std::string name, std::string source)
    : id_(id), name_(std::move(name)), source_(std::move(source)) {}

}  // namespace mewo::impl
