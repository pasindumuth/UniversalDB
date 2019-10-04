#include "UniversalException.h"

namespace uni {
namespace assert {

UniversalException::UniversalException(
    const std::string& message)
    : _message(message) {}

const char* UniversalException::what() const noexcept {
  return _message.c_str();
}

} // namespace assert
} // namespace uni
