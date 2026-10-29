#pragma once

namespace mewo {

/**
 * https://en.cppreference.com/w/cpp/utility/variant/visit2.html#Example
 */
template <class... Ts>
struct Match : Ts... {
  using Ts::operator()...;
};

}  // namespace mewo
