#include <sstream>
#include <string>

namespace uni {
namespace utils {

// splits the string by \n, and indents every segment, and adds the \n back
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

} // namespace utils
} // namespace uni
