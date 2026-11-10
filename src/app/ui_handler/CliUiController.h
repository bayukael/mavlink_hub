#pragma once

#include "app/app_service/IAppService.h"
#include "app/ui_handler/CliCommandDescriptor.h"
#include "app/types/CommandResult.h"

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  /// Bounds and step for the live report refresh period (milliseconds), driven by
  /// the '-' / '+' buttons in the CLI. The default (and startup) period is
  /// kDefaultReportRefreshPeriodMs; each button press steps by 100 ms and clamps
  /// to [kMinReportRefreshPeriodMs, kMaxReportRefreshPeriodMs].
  inline constexpr int kDefaultReportRefreshPeriodMs = 1000;
  inline constexpr int kMinReportRefreshPeriodMs = 100;
  inline constexpr int kMaxReportRefreshPeriodMs = 5000;
  inline constexpr int kReportRefreshPeriodStepMs = 100;

  /// One row in an overview table (endpoint list or agent list). The fields map
  /// to the columns rendered by the CLI view: name, type, and status for both;
  /// report is only meaningful for agents (endpoints have no report and leave it
  /// empty). The view renders these as-is; it neither parses nor reformats the
  /// report text.
  struct OverviewRow
  {
    std::string name;
    std::string type;
    std::string status;
    std::string report;
  };

  // Holds the state and actions of the CLI UI, independent of any rendering
  // framework. This is the unit-testable core: it loads command descriptors
  // from an IAppService, tracks the selected/committed command and payload,
  // executes commands asynchronously against the service, and stores the
  // resulting CommandResult. It owns no UI; a view (e.g. CliUiHandler) binds
  // its component tree to this controller.
  class CliUiController
  {
  public:
    /// @param appsrv the service commands are dispatched to. Must outlive this controller.
    explicit CliUiController(IAppService& appsrv);
    ~CliUiController();
    CliUiController(const CliUiController&) = delete;
    CliUiController& operator=(const CliUiController&) = delete;
    CliUiController(CliUiController&&) noexcept;
    CliUiController& operator=(CliUiController&&) noexcept;

    /// @return the descriptors for the eight user-executable commands, in menu
    ///         display order; each entry is a `CliCommandDescriptor` derived
    ///         from `appServiceCommandDescriptors()` by indexing with its own
    ///         `AppServiceCommandType`. This is the single command container held
    ///         by the controller - each entry carries its own dispatch type and
    ///         display name, so the menu needs no separate index map and no
    ///         parallel names container. A `std::array` is used because the menu
    ///         membership is fixed and known up front.
    const std::array<CliCommandDescriptor, 8>& commandDescriptors() const;

    /// @return index of the currently highlighted (but not necessarily committed) command.
    int selectedCommand() const;

    /// Mutable access to the selected-command index, for the view to bind against.
    int& selectedCommand();

    /// Set which command is highlighted.
    void selectCommand(int index);

    /// @return index of the committed command, or -1 if none is committed.
    int committedCommand() const;

    /// Mark \p index as the command to execute on the next executeCurrentCommand.
    void commitCommand(int index);

    /// @return the current command payload.
    const std::string& payload() const;

    /// Mutable access to the payload buffer, for the view to bind against.
    std::string& payload();

    /// Replace the payload.
    void setPayload(std::string value);

    /// Clear the payload buffer.
    void clearPayload();

    /// @return true while a command is being executed in the background.
    bool executing() const;

    /// @return the name of the command most recently executed.
    std::string executedCommand() const;

    /// @return the result of the most recently executed command, or std::nullopt if none.
    std::optional<CommandResult> commandResult() const;

    /// Clear the stored command result.
    void clearResult();

    /// @return a snapshot copy of the rows to render in the Agent List overview
    ///         table, taken under the overview mutex so reads are race-free even
    ///         while a periodic refresh worker is repopulating them. The view
    ///         consumes this as-is.
    std::vector<OverviewRow> agentRows() const;

    /// @return a snapshot copy of the rows to render in the Endpoint List
    ///         overview table, taken under the overview mutex so reads are
    ///         race-free even while a periodic refresh worker is repopulating
    ///         them. The view consumes this as-is.
    std::vector<OverviewRow> endpointRows() const;

    // Registers a callback invoked (from the worker thread) whenever the
    // observable state changes asynchronously, e.g. when a command finishes
    // executing or a periodic report refresh completes. The view uses this to
    // request a UI redraw.
    void setOnUpdate(std::function<void()> callback);

    // Refreshes the overview agent/endpoint rows from the app service's current
    // GET_AGENT_STATUS_ALL / GET_AGENT_REPORT_ALL / GET_ENDPOINT_STATUS_ALL
    // results, so the overview reflects hub state that already exists (e.g. a
    // plan applied on startup) and each agent's report. This is a programmatic
    // fetch: it calls executeCommand directly and does NOT go through the
    // committed command / [Execute Command] execution path (no command queue is
    // involved), and it leaves the user-command execution state untouched. Not
    // tied to startup: the periodic refresh (startPeriodicRefresh) and the
    // startup population both drive this same method.
    void updateOverviewData();

    /// @return the current report refresh period in milliseconds (default 1000).
    int reportRefreshPeriod() const;

    /// @return true when the report refresh period is at its minimum bound
    ///         (100 ms), in which case the '-' button is disabled.
    bool reportRefreshPeriodAtMin() const;

    /// @return true when the report refresh period is at its maximum bound
    ///         (5000 ms), in which case the '+' button is disabled.
    bool reportRefreshPeriodAtMax() const;

    /// Increase the report refresh period by 100 ms (no change at the maximum).
    void increaseReportRefreshPeriod();

    /// Decrease the report refresh period by 100 ms (no change at the minimum).
    void decreaseReportRefreshPeriod();

    // Starts the periodic report refresh worker thread. The thread periodically
    // calls updateOverviewData() (re-querying agent reports and refreshing the
    // display rows) every reportRefreshPeriod() milliseconds, then invokes the
    // registered on_update callback so the view redraws. Adjusting the period
    // while running takes effect on the next cycle. No-op if already running.
    void startPeriodicRefresh();

    // Stops the periodic report refresh worker thread and waits for it to
    // finish. No-op if it is not running. Safe to call even when nothing is
    // running.
    void stopPeriodicRefresh();

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
