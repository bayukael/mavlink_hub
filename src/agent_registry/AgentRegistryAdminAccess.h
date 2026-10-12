#pragma once
#include "agent_registry/AgentRegistryUserAccess.h"

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <memory>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  /// Read/write access to a registry of agent definitions.
  ///
  /// Agent definitions are stored by name (the \c key). Admin access can add/remove
  /// definitions and mint read-only user handles for consumers.
  class AgentRegistryAdminAccess : public AgentRegistryUserAccess
  {
  public:
    using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

    virtual ~AgentRegistryAdminAccess() = default;

    /// Register \p def under \p key.
    /// @return false if \p key is already registered (the existing entry is kept).
    virtual bool addAgentDefinition(const std::string& key, const AgentDefinition&) = 0;

    /// Unregister the definition under \p key.
    /// @return false if \p key is not registered.
    virtual bool removeAgentDefinition(const std::string& key) = 0;

    /// @return a read-only view over the current definitions, decoupled from later
    ///         admin mutations of this registry.
    virtual std::unique_ptr<AgentRegistryUserAccess> createUser() = 0;
  };
} // namespace pendarlab::app::mavlink_hub