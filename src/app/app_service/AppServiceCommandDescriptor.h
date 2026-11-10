#pragma once

#include <array>
#include <cstddef>
#include <string_view>

namespace pendarlab::app::mavlink_hub
{
  /// The set of user commands the app service understands.
  ///
  /// The enumerators must stay aligned with the canonical kCommands descriptor
  /// table in AppServiceCommandDescriptor.cpp:
  ///  - Adding a command means editing BOTH this enum and kCommands, inserting
  ///    the new command in the same position/order in both places.
  ///  - The first enumerator must keep value 0 (LOAD_PLAN_FROM_PATH remains the
  ///    zero-th command); this is enforced by a static_assert in the .cpp.
  ///  - The relative order of existing enumerators must not change.
  enum class AppServiceCommandType {
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
    GET_AGENT_REPORT,
    GET_AGENT_REPORT_ALL,
  };

  /// Number of user commands, i.e. the size of the canonical descriptor table.
  /// Kept in sync with `AppServiceCommandType` (the table has one entry per
  /// enumerator, in enum order); adding a command grows this automatically.
  inline constexpr std::size_t kAppServiceCommandCount =
      static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT_ALL) + 1;

  /// Static metadata for one user command, surfaced to the CLI UI.
  struct AppServiceCommandDescriptor {
    AppServiceCommandType type;          ///< The command this descriptor describes.
    std::string_view name;         ///< Canonical command name as typed in the UI.
    bool requires_payload;         ///< Whether the command needs a payload to execute.
    std::string_view payload_hint; ///< Human-readable hint for the expected payload format.
    std::string_view description;  ///< Human-readable description of the command, at most 150 characters.
  };

  // Canonical descriptor table for all commands, indexed by AppServiceCommandType
  // (one entry per enumerator, in enum order). Single source of truth for the
  // command names and payload metadata surfaced to the CLI UI.
  //
  // Keep this table aligned with AppServiceCommandType (AppServiceCommandDescriptor.h):
  // adding a command requires editing the enum and this table together, in the
  // same order, keeping the first enumerator (LOAD_PLAN_FROM_PATH) at value 0.
  const std::array<AppServiceCommandDescriptor, kAppServiceCommandCount>& appServiceCommandDescriptors();

} // namespace pendarlab::app::mavlink_hub
