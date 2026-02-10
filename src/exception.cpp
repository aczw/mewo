#include "exception.hpp"

namespace mewo {

Exception::Exception(const std::string& message)
    : std::runtime_error(message)
{
}

}
