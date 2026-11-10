#include "app/ui_handler/CliUiController.h"

#include "common/json_utils/JsonUtils.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <utility>

namespace pendarlab::app::mavlink_hub
{
  struct CliUiController::CliUiControllerImpl {
    explicit CliUiControllerImpl(IAppService& appsrv);

    IAppService& app_service;

    std::array<CliCommandDescriptor, 8> command_descriptors;
    int selected_command = 0;
    int committed_command = -1;

    std::string payload;
    bool executing = false;
    std::string executed_command;
    std::optional<CommandResult> command_result;
    std::mutex result_mutex;
    std::thread execute_thread;
    std::function<void()> on_update;

    // Overview rows. Guarded by rows_mutex: updateOverviewData() (called both at
    // startup and periodically on the worker thread) swaps complete new vectors
    // in under the lock, and agentRows()/endpointRows() return a snapshot copy
    // under the lock, so reads and writes never race.
    std::mutex rows_mutex;
    std::vector<OverviewRow> agent_rows;
    std::vector<OverviewRow> endpoint_rows;

    // Report refresh period (milliseconds), in [kMinReportRefreshPeriodMs,
    // kMaxReportRefreshPeriodMs], default kDefaultReportRefreshPeriodMs.
    // Guarded by period_mutex; changing it notifies the refresh condition
    // variable so the worker adopts the new cadence promptly.
    std::mutex period_mutex;
    int report_refresh_period_ms = kDefaultReportRefreshPeriodMs;

    // Periodic report refresh worker thread. The worker waits on refresh_cv with
    // a timeout of the current period, then calls updateOverviewData() and the
    // on_update callback. refresh_stop + a notify are used to stop it cleanly.
    std::atomic<bool> refresh_stop{ false };
    std::mutex refresh_mutex;
    std::condition_variable refresh_cv;
    std::thread refresh_thread;

    bool validCommittedCommand() const;
  };

  CliUiController::CliUiControllerImpl::CliUiControllerImpl(IAppService& appsrv)
      : app_service(appsrv),
        // Build the fixed-size menu as a derived view of the canonical
        // appServiceCommandDescriptors() table: each user-executable command is
        // selected by casting its AppServiceCommandType to std::size_t and used
        // as the index into the table, then wrapped in a CliCommandDescriptor.
        // This is the controller's single self-describing command container
        // (std::array, fixed size known up front): each entry carries its own
        // type, display name, and payload metadata, so the menu-to-command
        // mapping (cmd_type = command_descriptors[committed].type) needs no
        // separate index map and no parallel names container. The initializer
        // list runs before the body so no default constructor for
        // CliCommandDescriptor is needed. Only these eight commands are offered
        // in the menu; plan lifecycle, endpoint/agent mutation, GET_STATUS, and
        // GET_CURRENT_PLAN stay supported by AppService but are not shown.
        command_descriptors{
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_ENDPOINT_LIST)]),
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_ENDPOINT_STATUS)]),
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_ENDPOINT_STATUS_ALL)]),
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_LIST)]),
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_STATUS)]),
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_STATUS_ALL)]),
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT)]),
          CliCommandDescriptor(appServiceCommandDescriptors()[static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT_ALL)]),
        }
  {
  }

  bool CliUiController::CliUiControllerImpl::validCommittedCommand() const
  {
    return committed_command >= 0 && committed_command < static_cast<int>(command_descriptors.size());
  }

  
  CliUiController::CliUiController(IAppService& appsrv) : d(std::make_unique<CliUiControllerImpl>(appsrv))
  {
  }
  CliUiController::~CliUiController()
  {
    stopPeriodicRefresh();
    joinExecution();
  }
  CliUiController::CliUiController(CliUiController&&) noexcept = default;
  CliUiController& CliUiController::operator=(CliUiController&&) noexcept = default;

  const std::array<CliCommandDescriptor, 8>& CliUiController::commandDescriptors() const
  {
    return d->command_descriptors;
  }

  int CliUiController::selectedCommand() const
  {
    return d->selected_command;
  }
  int& CliUiController::selectedCommand()
  {
    return d->selected_command;
  }
  void CliUiController::selectCommand(int index)
  {
    d->selected_command = index;
  }
  int CliUiController::committedCommand() const
  {
    return d->committed_command;
  }
  void CliUiController::commitCommand(int index)
  {
    d->committed_command = index;
  }

  const std::string& CliUiController::payload() const
  {
    return d->payload;
  }
  std::string& CliUiController::payload()
  {
    return d->payload;
  }
  void CliUiController::setPayload(std::string value)
  {
    d->payload = std::move(value);
  }
  void CliUiController::clearPayload()
  {
    d->payload.clear();
  }

  bool CliUiController::executing() const
  {
    std::lock_guard<std::mutex> lock(d->result_mutex);
    return d->executing;
  }
  std::string CliUiController::executedCommand() const
  {
    std::lock_guard<std::mutex> lock(d->result_mutex);
    return d->executed_command;
  }
  std::optional<CommandResult> CliUiController::commandResult() const
  {
    std::lock_guard<std::mutex> lock(d->result_mutex);
    return d->command_result;
  }
  void CliUiController::clearResult()
  {
    std::lock_guard<std::mutex> lock(d->result_mutex);
    d->command_result.reset();
  }

  std::vector<OverviewRow> CliUiController::agentRows() const
  {
    std::lock_guard<std::mutex> lock(d->rows_mutex);
    return d->agent_rows;
  }

  std::vector<OverviewRow> CliUiController::endpointRows() const
  {
    std::lock_guard<std::mutex> lock(d->rows_mutex);
    return d->endpoint_rows;
  }

  void CliUiController::updateOverviewData()
  {
    // Programmatic fetch of the agents'/endpoints' list, status, and reports.
    // This calls executeCommand directly (synchronously) rather than through the
    // committed-command path, so it neither enqueues commands nor touches the
    // committed/selected command, payload, or result state. Called at UI start
    // and periodically by the report-refresh worker thread; both drive this same
    // report-surfacing path, so on-demand and periodic refreshes never conflict.
    AppServiceCommand agent_cmd;
    agent_cmd.cmd_type = AppServiceCommandType::GET_AGENT_STATUS_ALL;
    const CommandResult agent_result = d->app_service.executeCommand(agent_cmd);
    if (agent_result.success) {
      auto entries = json_utils::agentStatusListFromJsonString(agent_result.data);
      if (entries.has_value()) {
        std::vector<OverviewRow> rows;
        rows.reserve(entries->size());
        for (auto& [name, status] : *entries) {
          OverviewRow row;
          row.name = std::move(name);
          row.status = std::move(status);
          // Type is intentionally left blank: no layer stores an agent type yet.
          rows.push_back(std::move(row));
        }
        std::lock_guard<std::mutex> lock(d->rows_mutex);
        d->agent_rows = std::move(rows);
      }
    }

    // Fetch every registered agent's report and attach it to the matching agent
    // row by name. Endpoints have no report (only agents do), so only the agent
    // table gets a Report populated. The report string is opaque; it is shown
    // as-is.
    AppServiceCommand report_cmd;
    report_cmd.cmd_type = AppServiceCommandType::GET_AGENT_REPORT_ALL;
    const CommandResult report_result = d->app_service.executeCommand(report_cmd);
    if (report_result.success) {
      auto report_entries = json_utils::agentReportListFromJsonString(report_result.data);
      if (report_entries.has_value()) {
        std::unordered_map<std::string, std::string> report_by_name;
        report_by_name.reserve(report_entries->size());
        for (auto& [name, report] : *report_entries) {
          report_by_name[std::move(name)] = std::move(report);
        }
        std::lock_guard<std::mutex> lock(d->rows_mutex);
        for (OverviewRow& row : d->agent_rows) {
          auto it = report_by_name.find(row.name);
          if (it != report_by_name.end()) {
            row.report = it->second;
          }
        }
      }
    }

    AppServiceCommand endpoint_cmd;
    endpoint_cmd.cmd_type = AppServiceCommandType::GET_ENDPOINT_STATUS_ALL;
    const CommandResult endpoint_result = d->app_service.executeCommand(endpoint_cmd);
    if (endpoint_result.success) {
      auto entries = json_utils::endpointStatusListFromJsonString(endpoint_result.data);
      if (entries.has_value()) {
        std::vector<OverviewRow> rows;
        rows.reserve(entries->size());
        for (auto& [name, status] : *entries) {
          OverviewRow row;
          row.name = std::move(name);
          row.status = std::move(status);
          rows.push_back(std::move(row));
        }
        std::lock_guard<std::mutex> lock(d->rows_mutex);
        d->endpoint_rows = std::move(rows);
      }
    }
  }

  int CliUiController::reportRefreshPeriod() const
  {
    std::lock_guard<std::mutex> lock(d->period_mutex);
    return d->report_refresh_period_ms;
  }

  bool CliUiController::reportRefreshPeriodAtMin() const
  {
    return reportRefreshPeriod() <= kMinReportRefreshPeriodMs;
  }

  bool CliUiController::reportRefreshPeriodAtMax() const
  {
    return reportRefreshPeriod() >= kMaxReportRefreshPeriodMs;
  }

  void CliUiController::increaseReportRefreshPeriod()
  {
    bool notified = false;
    {
      std::lock_guard<std::mutex> lock(d->period_mutex);
      if (d->report_refresh_period_ms < kMaxReportRefreshPeriodMs) {
        d->report_refresh_period_ms += kReportRefreshPeriodStepMs;
        notified = true;
      }
    }
    // Wake the worker so it adopts the new cadence promptly.
    if (notified) {
      d->refresh_cv.notify_all();
    }
  }

  void CliUiController::decreaseReportRefreshPeriod()
  {
    bool notified = false;
    {
      std::lock_guard<std::mutex> lock(d->period_mutex);
      if (d->report_refresh_period_ms > kMinReportRefreshPeriodMs) {
        d->report_refresh_period_ms -= kReportRefreshPeriodStepMs;
        notified = true;
      }
    }
    if (notified) {
      d->refresh_cv.notify_all();
    }
  }

  void CliUiController::startPeriodicRefresh()
  {
    if (d->refresh_thread.joinable()) {
      return;
    }

    d->refresh_stop = false;

    // The periodic report-refresh worker: wait for the current period, then
    // re-query the agents' status and reports and repopulate the overview rows
    // (via the same updateOverviewData() path the startup population uses), and
    // notify the view so it redraws. The slow command I/O stays on this worker
    // thread, off the UI thread, so the UI never stalls.
    d->refresh_thread = std::thread([this]() {
      std::unique_lock<std::mutex> lock(d->refresh_mutex);
      while (!d->refresh_stop.load()) {
        const int period_ms = reportRefreshPeriod();
        if (d->refresh_cv.wait_for(lock, std::chrono::milliseconds(period_ms)) == std::cv_status::timeout) {
          // Time to refresh. Unlock while doing the work so a stop/period
          // change can proceed.
          lock.unlock();
          updateOverviewData();
          if (d->on_update) {
            d->on_update();
          }
          lock.lock();
        }
        // On a notify (period change or stop), loop around to re-read the period
        // (and the stop flag); only a timeout triggers an actual refresh.
      }
    });
  }

  void CliUiController::stopPeriodicRefresh()
  {
    {
      std::lock_guard<std::mutex> lock(d->refresh_mutex);
      d->refresh_stop = true;
    }
    d->refresh_cv.notify_all();
    if (d->refresh_thread.joinable()) {
      d->refresh_thread.join();
    }
  }

  void CliUiController::setOnUpdate(std::function<void()> callback)
  {
    d->on_update = std::move(callback);
  }

    void CliUiController::executeCurrentCommand()
  {
    if (!d->validCommittedCommand()) {
      return;
    }

    AppServiceCommand cmd;
    cmd.cmd_type = d->command_descriptors[static_cast<std::size_t>(d->committed_command)].type;
    cmd.payload = d->payload;

    {
      std::lock_guard<std::mutex> lock(d->result_mutex);
      if (d->executing) {
        return;
      }
      d->executing = true;
      d->executed_command = d->command_descriptors[static_cast<std::size_t>(d->committed_command)].name;
    }

    // Responds to the [Execute Command] button: this is the single command
    // execution entry point and the only caller of the worker execution path.
    // The worker thread runs the committed command on the app service, guards
    // the result, and reports back through on_update.
    joinExecution();

    d->execute_thread = std::thread([this, cmd]() {
      CommandResult result = d->app_service.executeCommand(cmd);
      {
        std::lock_guard<std::mutex> lock(d->result_mutex);
        d->command_result = std::move(result);
        d->executing = false;
      }
      if (d->on_update) {
        d->on_update();
      }
    });
  }

  void CliUiController::joinExecution()
  {
    if (d->execute_thread.joinable()) {
      d->execute_thread.join();
    }
  }

} // namespace pendarlab::app::mavlink_hub
