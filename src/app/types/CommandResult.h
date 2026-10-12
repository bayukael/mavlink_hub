#pragma once

#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  /// Outcome of executing a UserCommand against the app service.
  struct CommandResult {
    bool success;                     ///< Whether the command succeeded.
    std::vector<std::string> message; ///< Human-readable messages describing the outcome.
    std::string data;                 ///< Optional structured (typically JSON) data produced by the command.
  };
} // namespace pendarlab::app::mavlink_hub
