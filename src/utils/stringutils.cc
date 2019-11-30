#include "stringutils.h"

namespace uni {
namespace utils {
namespace string {

std::string indent(std::string s, int indent) {
  auto ss = std::stringstream();
  auto i = 0;
  for (auto j = 0; j < s.length(); j++) {
    if (s.at(j) == '\n') {
      ss << std::string(indent, ' ') << s.substr(i, j - i) << std::endl;
      i = j + 1;
    }
  }
  return ss.str();
}

} // namespace string
} // namespace utils
} // namespace uni
