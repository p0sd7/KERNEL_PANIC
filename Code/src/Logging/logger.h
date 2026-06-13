#pragma once

#include <string>

namespace kernel {
namespace logging {

void init(const std::string& log_path);
void shutdown();
void logError(const std::string& message);
void logWarning(const std::string& message);
void logInfo(const std::string& message);

}  // namespace logging
}  // namespace kernel