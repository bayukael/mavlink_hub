#pragma once
#include "agent_registry/AgentRegistryUserAccess.h"

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <memory>
#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  class AgentRegistryAdminAccess : public AgentRegistryUserAccess
  {
  public:
    using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

    virtual ~AgentRegistryAdminAccess() = default;

    virtual bool addAgentDefinition(const std::string& key, const AgentDefinition&) = 0;
    virtual bool removeAgentDefinition(const std::string& key) = 0;
    virtual std::unique_ptr<AgentRegistryUserAccess> createUser() = 0;
  };
} // namespace pendarlab::app::mavlink_hub