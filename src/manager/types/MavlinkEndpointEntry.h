#pragma once

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  /// Describes one MAVLink endpoint to be created within a UserPlan.
  struct MavlinkEndpointEntry {
    std::string type;                                    ///< Registered transport type name used to connect the endpoint.
    std::unordered_map<std::string, std::string> config; ///< Transport-specific config, validated by the transport's config parser.
    bool connect_on_create;                              ///< If true, the manager connects the endpoint as part of plan execution.
  };

} // namespace pendarlab::app::mavlink_hub
