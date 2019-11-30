#ifndef UNI_UTILS_STRING_H
#define UNI_UTILS_STRING_H

#include <sstream>
#include <string>

namespace uni {
namespace utils {
namespace string {

// Splits the string by \n, and indents every segment, and adds the \n back
std::string indent(std::string s, int indent);

} // namespace string
} // namespace utils
} // namespace uni

#endif // UNI_UTILS_STRING_H
