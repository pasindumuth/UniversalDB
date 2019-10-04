#include <iostream>
#include <tuple>
#include <vector>

std::tuple<std::string, std::string> parse_endpoint(std::string endpoint) {
  int delim_pos = endpoint.find(':');
  std::string hostname(endpoint.substr(0, delim_pos));
  std::string port(endpoint.substr(delim_pos + 1, endpoint.size()));
  return {hostname, port};
}

std::vector<std::tuple<std::string, std::string>> parse_args(int argc, char* argv[]) {
  std::vector<std::tuple<std::string, std::string>> endpoints;
  for (int i = 1; i < argc; i++) {
    endpoints.push_back(parse_endpoint(std::string(argv[i])));
  }
  return endpoints;
}
