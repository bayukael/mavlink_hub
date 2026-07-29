#pragma once

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  struct AgentEntry{
    std::string type;
    std::unordered_map<std::string, std::string> config;
  };
} // namespace pendarlab::app::mavlink_hub
