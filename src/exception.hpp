#pragma once

#include <format>
#include <stdexcept>
#include <string>
#include <utility>

namespace mewo {

class Exception : public std::runtime_error {
  public:
  explicit Exception(const std::string& message);

  template <typename... Args>
  Exception(std::format_string<Args...> format, Args&&... args)
      : std::runtime_error(std::format(format, std::forward<Args>(args)...))
  {
  }
};

}
