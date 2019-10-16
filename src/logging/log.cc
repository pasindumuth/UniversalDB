#include <logging/log.h>

namespace uni {
namespace logging {

Level& get_log_level() {
  static Level log_level;
  return log_level;
}

} // namespace uni
} // namespace logging
