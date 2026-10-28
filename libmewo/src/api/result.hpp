#pragma once

#include "utility/error.hpp"

#include <mewo/mewo.h>

namespace mewo {

mewo_result_t error_to_result(Error error);

}  // namespace mewo
