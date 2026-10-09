#pragma once

#include <mavlink_hub_sdk/agent/AgentDefinition.h>
#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  /// Read-only access to a registry of agent definitions, for consumers that must not
  /// mutate it. Lookups are by the registration key (definition name).
  class AgentRegistryUserAccess
  {
  public:
    using AgentDefinition = pendarlab::sdk::mavlink_hub::AgentDefinition;

    virtual ~AgentRegistryUserAccess() = default;

    /// Look up a registered definition by \p key.
    /// @return pointer to the definition, or nullptr if \p key is not registered.
    /// @warning The returned pointer may be null; always check before dereferencing.
    virtual const AgentDefinition* operator[](const std::string& key) const = 0; // Be careful when it returns nullptr

    /// @return the registration keys of all currently registered definitions.
    virtual std::vector<std::string> showRegistered() const = 0;

    /// @return true if a definition is registered under \p key.
    virtual bool isRegistered(const std::string& key) const = 0;
  };
} // namespace pendarlab::app::mavlink_hub