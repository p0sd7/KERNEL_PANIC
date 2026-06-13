#include "logger.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>

namespace kernel {
namespace logging {

namespace {
std::ofstream log_file;
std::mutex log_mutex;
bool enabled = false;

std::string currentTime() {
  auto now = std::chrono::system_clock::now();
  auto time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

void writeLog(const std::string& level, const std::string& message) {
  if (!enabled) return;
  std::lock_guard<std::mutex> lock(log_mutex);
  if (log_file.is_open()) {
    log_file << "[" << currentTime() << "] [" << level << "] " << message
             << std::endl;
    log_file.flush();
  }
}
}  // anonymous namespace

void init(const std::string& log_path) {
  std::filesystem::path p(log_path);
  auto parent = p.parent_path();
  if (!parent.empty() && !std::filesystem::exists(parent)) {
    std::error_code ec;
    std::filesystem::create_directories(parent, ec);
    if (ec) {
      enabled = false;
      return;
    }
  }
  log_file.open(log_path, std::ios::out | std::ios::app);
  enabled = log_file.is_open();
}

void shutdown() {
  if (log_file.is_open()) {
    log_file.close();
  }
  enabled = false;
}

void logError(const std::string& message) { writeLog("ERROR", message); }
void logWarning(const std::string& message) { writeLog("WARNING", message); }
void logInfo(const std::string& message) { writeLog("INFO", message); }

}  // namespace logging
}  // namespace kernel