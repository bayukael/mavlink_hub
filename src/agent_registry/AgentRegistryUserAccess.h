#pragma once

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  class AgentRegistryUserAccess
  {
  public:
    using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

    virtual ~AgentRegistryUserAccess() = default;

    virtual const AgentDefinition* operator[](const std::string& key) const = 0; // Be careful when it returns nullptr
    virtual std::vector<std::string> showRegistered() const = 0;
    virtual bool isRegistered(const std::string& key) const = 0;
  };
} // namespace pendarlab::app::mavlink_hub