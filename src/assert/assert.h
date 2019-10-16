#ifndef UNI_ASSERT_ASSERT_H
#define UNI_ASSERT_ASSERT_H

#include <sstream>
#include <string>

#include <assert/UniversalException.h>

namespace uni {
namespace assert {

// Macros used to assert preconditions to functions. These can be
// repaced by empty macros when doing the production build.

#define UNIVERSAL_ASSERT_MESSAGE(EXPRESSION, MESSAGE)          \
if (!(EXPRESSION)) {                                           \
  std::stringstream message;                                   \
  message << "Assertion Failed: "                              \
    << "file: " << __FILE__                                    \
    << " function: " << __FUNCTION__                           \
    << " line: " << __LINE__                                   \
    << " expression: " << #EXPRESSION                          \
    << " message: " << MESSAGE                                 \
    << std::endl;                                              \
  throw uni::assert::UniversalException(message.str());        \
}

#define UNIVERSAL_TERMINATE(MESSAGE) UNIVERSAL_ASSERT_MESSAGE(false, MESSAGE)

} // namespace assert
} // namespace uni

#endif // UNI_ASSERT_ASSERT_H
