#pragma once

#include "agent_registry/AgentRegistryAdminAccess.h"

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <memory>
#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  class AgentRegistry : public AgentRegistryAdminAccess
  {
  public:
    using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

    AgentRegistry();
    ~AgentRegistry();
    AgentRegistry(AgentRegistry&&) noexcept;
    AgentRegistry& operator=(AgentRegistry&&) noexcept;

    virtual bool addAgentDefinition(const std::string& key, const AgentDefinition&) override;
    virtual bool removeAgentDefinition(const std::string& key) override;
    virtual std::unique_ptr<AgentRegistryUserAccess> createUser() override;
    virtual const AgentDefinition* operator[](const std::string& key) const override; // Be careful when it returns nullptr
    virtual std::vector<std::string> showRegistered() const override;
    virtual bool isRegistered(const std::string& key) const override;

  private:
    struct AgentRegistryImpl;
    std::unique_ptr<AgentRegistryImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub
