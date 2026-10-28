#include "api/result.hpp"

#include <utility>

namespace mewo {

mewo_result_t error_to_result(Error error) {
  switch (error) {
    case Error::kInstanceCreationFailed: return MEWO_ERROR_INTERNAL;
  }

  std::unreachable();
}

}  // namespace mewo
