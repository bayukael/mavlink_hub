#pragma once
#include "agent_registry/AgentRegistryUserAccess.h"

#include <functional>
#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  /// Read-only view over a specific agent-definition map.
  ///
  /// Holds a reference to the same map that the owning AgentRegistry maintains, so it stays
  /// in sync with subsequent admin mutations. Intended for consumers that only need to
  /// look definitions up.
  class AgentRegistryUser : public AgentRegistryUserAccess
  {
  public:
    using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

    /// @param registry the definition map to view. Must outlive this object.
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