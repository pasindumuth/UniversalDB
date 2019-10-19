#include <string>
#include <tuple>
#include <vector>

#include <constants/constants.h>

uni::constants::Constants initialize_constants() {
  return uni::constants::Constants(1, 1610, 1710, 1810, 1000, 4, 1000);
}

std::vector<std::string> parse_hostnames(int argc, char* argv[]) {
  auto hostnames = std::vector<std::string>();
  for (int i = 1; i < argc; i++) {
    hostnames.push_back(argv[i]);
  }
  return hostnames;
}
