#ifndef UNI_UTILS
#define UNI_UTILS

#include <string>
#include <tuple>
#include <vector>

#include <common/common.h>
#include <constants/constants.h>

uni::constants::Constants initialize_constants() {
  return uni::constants::Constants(2, 1610, 1710, 1810, 1000, 4, 1000);
}

std::vector<std::string> parse_args(int32_t argc, char* argv[]) {
  auto args = std::vector<std::string>();
  for (auto i = 1; i < argc; i++) {
    args.push_back(argv[i]);
  }
  return args;
}

#endif // UNI_UTILS
