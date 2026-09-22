#pragma once

#include "app/app_service/IAppService.h"
#include "app/types/CommandDescriptor.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  // Holds the state and actions of the CLI UI, independent of any rendering
  // framework. This is the unit-testable core: it loads command descriptors
  // from an IAppService, tracks the selected/committed command and payload,
  // executes commands asynchronously against the service, and stores the
  // resulting CommandResult. It owns no UI; a view (e.g. CliUiHandler) binds
  // its component tree to this controller.
  class CliUiController
  {
  public:
    explicit CliUiController(IAppService& appsrv);
    ~CliUiController();
    CliUiController(const CliUiController&) = delete;
    CliUiController& operator=(const CliUiController&) = delete;
    CliUiController(CliUiController&&) noexcept;
    CliUiController& operator=(CliUiController&&) noexcept;

    const std::vector<CommandDescriptor>& commandDescriptors() const;
    const std::vector<std::string>& commandEntries() const;
    std::vector<std::string>& commandEntries();

    int selectedCommand() const;
    int& selectedCommand();
    void selectCommand(int index);
    int committedCommand() const;
    void commitCommand(int index);

    const std::string& payload() const;
    std::string& payload();
    void setPayload(std::string value);
    void clearPayload();

    bool executing() const;
    std::string executedCommand() const;
    std::optional<CommandResult> commandResult() const;
    void clearResult();

    // Starts an async execution of the committed command with the current
    // payload. No-op while already executing or if no command is committed.
    void executeCurrentCommand();

    // Blocks until any in-flight execution finishes. Safe to call even when
    // nothing is executing.
    void joinExecution();

  private:
    struct CliUiControllerImpl;
    std::unique_ptr<CliUiControllerImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
