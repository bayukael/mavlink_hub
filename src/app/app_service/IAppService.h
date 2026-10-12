#pragma once

#include "app/types/CommandDescriptor.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"

namespace pendarlab::app::mavlink_hub
{
  /// Service facade that turns user commands into manager/loader operations.
  ///
  /// The service holds application-level state (such as the current user plan) and mediates
  /// between incoming UserCommands and the manager/lib loader. The CLI UI talks to the
  /// application exclusively through this interface.
  class IAppService
  {
  public:
    virtual ~IAppService() = default;

    /// Execute \p cmd and return its result.
    ///
    /// Commands that produce structured data (e.g. checking or applying a plan) put that data
    /// in CommandResult::data as a JSON string; \c success is true only if the command itself
    /// was dispatched successfully, which is not necessarily the same as the underlying
    /// operation succeeding for every entry.
    /// @param cmd the command to execute.
    /// @return the command outcome.
    virtual CommandResult executeCommand(const UserCommand& cmd) = 0;

    /// @return the canonical descriptor table for all supported commands.
    virtual std::vector<CommandDescriptor> getCommandDescriptors() const { return commandDescriptors(); }
  };
} // namespace pendarlab::app::mavlink_hub
