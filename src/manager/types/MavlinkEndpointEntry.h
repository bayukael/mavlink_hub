#pragma once

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  struct MavlinkEndpointEntry{
    std::string type;
    std::unordered_map<std::string, std::string> config;
    bool connect_on_create;
  };
  
} // namespace pendarlab::app::mavlink_hub
