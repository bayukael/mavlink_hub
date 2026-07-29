#pragma once

#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  struct CommandResult {
    bool success;
    std::vector<std::string> message;
    std::string data;
  };
} // namespace pendarlab::app::mavlink_hub
