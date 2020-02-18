#ifndef UNI_LOGGING_LOG_H
#define UNI_LOGGING_LOG_H

#include <iostream>

#include <common/common.h>

namespace uni {
namespace logging {

// Log Levels
enum Level {
  FATAL = 0,
  ERROR = 1,
  WARN = 2,
  INFO = 3,
  DEBUG = 4,
  TRACE1 = 5,
  TRACE2 = 6
};

static const char* LevelNames[] = { "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE1", "TRACE2" };

Level& get_log_level();

// Macro used for logging. Can be turned off for production, or the logs
// can be redirected to different sources depending on the environment
// of deployment

#define LOG(LEVEL, MESSAGE) \
  if ((LEVEL) <= uni::logging::get_log_level()) { \
    std::cout << "Log " << uni::logging::LevelNames[(LEVEL)] << ": " << MESSAGE << std::endl; \
  }

} // namespace logging
} // namespace uni

#endif // UNI_LOGGING_LOG_H
