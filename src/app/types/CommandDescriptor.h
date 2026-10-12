#pragma once

#include <string_view>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  /// The set of user commands the app service understands.
  enum class UserCommandType {
    LOAD_PLAN_FROM_PATH,
    LOAD_PLAN_FROM_JSON_TEXT,
    GET_CURRENT_PLAN,
    CHECK_CURRENT_PLAN,
    APPLY_CURRENT_PLAN,
    DELETE_CURRENT_PLAN,
    GET_STATUS,
    ADD_MAVLINK_ENDPOINT,
    EDIT_MAVLINK_ENDPOINT,
    CONNECT_MAVLINK_ENDPOINT,
    DISCONNECT_MAVLINK_ENDPOINT,
    REMOVE_MAVLINK_ENDPOINT,
    GET_ENDPOINT_LIST,
    GET_ENDPOINT_STATUS,
    GET_ENDPOINT_STATUS_ALL,
    ADD_AGENT,
    EDIT_AGENT,
    START_AGENT,
    STOP_AGENT,
    REMOVE_AGENT,
    GET_AGENT_LIST,
    GET_AGENT_STATUS,
    GET_AGENT_STATUS_ALL,
  };

  /// Static metadata for one user command, surfaced to the CLI UI.
  struct CommandDescriptor {
    UserCommandType type;          ///< The command this descriptor describes.
    std::string_view name;         ///< Canonical command name as typed in the UI.
    bool requires_payload;         ///< Whether the command needs a payload to execute.
    std::string_view payload_hint; ///< Human-readable hint for the expected payload format.
  };

  // Canonical descriptor table for all commands, indexed by UserCommandType
  // (one entry per enumerator, in enum order). Single source of truth for the
  // command names and payload metadata surfaced to the CLI UI.
  const std::vector<CommandDescriptor>& commandDescriptors();

} // namespace pendarlab::app::mavlink_hub
