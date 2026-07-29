#pragma once
#include "agent_registry/AgentRegistryUserAccess.h"

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  class AgentRegistryUser : public AgentRegistryUserAccess
  {
  public:
    using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

    AgentRegistryUser(const std::unordered_map<std::string, std::reference_wrapper<const AgentDefinition>>& registry);
    ~AgentRegistryUser();
    AgentRegistryUser(AgentRegistryUser&&) noexcept;
    AgentRegistryUser& operator=(AgentRegistryUser&&) noexcept;

    virtual const AgentDefinition* operator[](const std::string& key) const override; // Be careful when it returns nullptr
    virtual std::vector<std::string> showRegistered() const override;
    virtual bool isRegistered(const std::string& key) const override;

    private:
    struct AgentRegistryUserImpl;
    std::unique_ptr<AgentRegistryUserImpl> d;
    
  };
} // namespace pendarlab::app::mavlink_hub