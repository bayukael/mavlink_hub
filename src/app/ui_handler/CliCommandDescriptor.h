#pragma once

#include "app/app_service/AppServiceCommandDescriptor.h"

#include <string>

namespace pendarlab::app::mavlink_hub
{
  /// UI-owned description of one user command shown in the CLI menu.
  ///
  /// Mirrors the members of `AppServiceCommandDescriptor` (`type`, `name`,
  /// `requires_payload`, `payload_hint`) but stores the name as an owned
  /// `std::string` (the app-service descriptor exposes a `std::string_view`
  /// into its canonical table) and adds a `description` string. Unlike the
  /// parallel `command_descriptors`/`command_entries` vectors that
  /// `CliUiController` used to keep index-aligned, a single vector of this
  /// struct makes each menu entry self-describing: the dispatch type, the
  /// rendered name, the payload metadata, and (when later populated) a
  /// human-readable description all travel together in one entry.
  ///
  /// The single constructor derives every member from an
  /// `AppServiceCommandDescriptor`. An optional `name` overrides the derived
  /// display name when it is non-empty; an empty `name` (the default) keeps the
  /// descriptor's name. `description` is left empty for now; wiring it from
  /// `AppServiceCommandDescriptor::description` is handled by a later task.
  struct CliCommandDescriptor
  {
    CliCommandDescriptor(const AppServiceCommandDescriptor& descriptor, std::string name = "");

    AppServiceCommandType type; ///< The command this describes (the dispatch type).
    std::string name;           ///< Display name rendered in the menu.
    bool requires_payload;      ///< Whether the command needs a payload to execute.
    std::string payload_hint;   ///< Human-readable hint for the expected payload format.
    std::string description;    ///< Human-readable description (empty for now).
  };

} // namespace pendarlab::app::mavlink_hub
