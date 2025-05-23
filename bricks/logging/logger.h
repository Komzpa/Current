#ifndef BRICKS_LOGGING_LOGGER_H
#define BRICKS_LOGGING_LOGGER_H

#include <fstream>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "../strings/printf.h"
#include "../util/singleton.h"

namespace current {
namespace logging {

class Logger {
 public:
  explicit Logger(const std::string& file_name = "current.log", size_t max_file_size = 10 * 1024 * 1024)
      : base_file_name_(file_name), max_file_size_(max_file_size) {
    Rotate();
  }

  void Log(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!file_.is_open()) {
      Rotate();
    }
    if (current_size_ + message.size() + 1 > max_file_size_) {
      Rotate();
    }
    file_ << message << '\n';
    file_.flush();
    current_size_ += message.size() + 1;
  }

 private:
  void Rotate() {
    if (file_.is_open()) {
      file_.close();
    }
    const std::string filename = base_file_name_ + "." + current::ToString(index_) + ".log";
    ++index_;
    file_.open(filename, std::ios::out | std::ios::app);
    current_size_ = static_cast<size_t>(file_.tellp());
  }

  std::string base_file_name_;
  size_t max_file_size_;
  size_t current_size_ = 0;
  size_t index_ = 0;
  std::ofstream file_;
  std::mutex mutex_;
};

class LoggerFactory {
 public:
  static Logger& Get(const std::string& name,
                     const std::string& file_name = "",
                     size_t max_file_size = 10 * 1024 * 1024) {
    auto& map = current::Singleton<std::map<std::string, std::unique_ptr<Logger>>>();
    auto& mtx = current::Singleton<std::mutex>();
    std::lock_guard<std::mutex> lock(mtx);
    auto it = map.find(name);
    if (it == map.end()) {
      std::string f = file_name.empty() ? (name + ".log") : file_name;
      it = map.emplace(name, std::make_unique<Logger>(f, max_file_size)).first;
    }
    return *it->second;
  }
};

}  // namespace logging
}  // namespace current

#endif  // BRICKS_LOGGING_LOGGER_H
