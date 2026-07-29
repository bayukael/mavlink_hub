#pragma once

#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  struct OperationResult {
    bool success = true;
    std::vector<std::string> messages;

    inline void merge(const OperationResult& other){
      success = success && other.success;
      messages.insert(messages.end(), other.messages.begin(), other.messages.end());
    }
  };
} // namespace pendarlab::app::mavlink_hub
