#pragma once
#include <map>
#include <optional>
#include <sstream>
#include <string>

class Config {
 public:
  Config(int argc, char** argv) {
    std::string key;
    for (int i = 1; i < argc; i++) {
      if (argv[i][0] == '-')
        key = std::string(&argv[i][1]);
      else {
        config_[key] = get(key).value_or("") + argv[i];
      }
    }
  }
  std::optional<std::string> get(const std::string& key) {
    auto it = config_.find(key);
    if (it != config_.end()) {
      return it->second;
    }
    return std::nullopt;
  }
  template <class T>
  std::optional<T> getAs(const std::string& key) {
    auto it = config_.find(key);
    if (it != config_.end()) {
      try {
        T val;
        std::istringstream iss(it->second);
        iss >> val;
        if (iss.fail()) return std::nullopt;
        return val;
      } catch (...) {
        return std::nullopt;
      }
    }
    return std::nullopt;
  }

 private:
  std::map<std::string, std::string> config_;
};