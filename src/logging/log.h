#ifndef UNI_LOGGING_LOG_H
#define UNI_LOGGING_LOG_H

#include <iostream>

namespace uni {
namespace logging {

// Log Levels
enum Level {
  INFO = 0,
  ERROR = 1,
  DEBUG = 2
};

static const char* LevelNames[] = { "INFO", "ERROR", "DEBUG" };

// Macros used for logging. Can be turned off for production, or the logs
// can be redirected to different sources depending on the environment
// of deployment

#define LOG(LEVEL, MESSAGE) \
  std::cout << "Log " << uni::logging::LevelNames[(LEVEL)] << ": " << (MESSAGE) << std::endl;

} // namespace logging
} // namespace uni

#endif // UNI_LOGGING_LOG_H
