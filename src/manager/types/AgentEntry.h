#pragma once

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  /// Describes one agent to be created within a UserPlan.
  struct AgentEntry {
    std::string type;                                    ///< Registered agent type name used to look up its definition.
    std::unordered_map<std::string, std::string> config; ///< Type-specific config, validated by the agent's config parser.
  };
} // namespace pendarlab::app::mavlink_hub
