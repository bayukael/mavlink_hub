#pragma once

#include "app/types/CommandDescriptor.h"

#include <string>

namespace pendarlab::app::mavlink_hub
{
  /// A user-facing command targeting the app service, with a string payload.
  ///
  /// The interpretation of \c payload depends on \c cmd_type; see CommandDescriptor for
  /// whether a payload is required and its expected shape.
  struct UserCommand {
    UserCommandType cmd_type;
    std::string payload;
  };

} // namespace pendarlab::app::mavlink_hub
