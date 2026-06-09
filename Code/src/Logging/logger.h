#pragma once

#include <string>

namespace kernel {
namespace logging {

void Init(const std::string& log_path = "kernel_panic.log");
void Shutdown();
void LogError(const std::string& message);
void LogWarning(const std::string& message);
void LogInfo(const std::string& message);

}  // namespace logging
}  // namespace kernel