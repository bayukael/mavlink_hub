#pragma once

#include "app/app_service/AppServiceCommandDescriptor.h"
#include "app/types/CommandResult.h"
#include "app/app_service/AppServiceCommand.h"

namespace pendarlab::app::mavlink_hub
{
  /// Service facade that turns user commands into manager/loader operations.
  ///
  /// The service holds application-level state (such as the current user plan) and mediates
  /// between incoming AppServiceCommands and the manager/lib loader. The CLI UI talks to the
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
    virtual CommandResult executeCommand(const AppServiceCommand& cmd) = 0;

    /// @return the canonical descriptor table for all supported commands.
    virtual std::vector<AppServiceCommandDescriptor> getCommandDescriptors() const {
      const auto& descriptors = appServiceCommandDescriptors();
      return std::vector<AppServiceCommandDescriptor>(descriptors.begin(), descriptors.end());
    }
  };
} // namespace pendarlab::app::mavlink_hub
