#include "app/ui_handler/CliUiController.h"

#include <functional>
#include <mutex>
#include <thread>

namespace pendarlab::app::mavlink_hub
{
  struct CliUiController::CliUiControllerImpl {
    explicit CliUiControllerImpl(IAppService& appsrv);

    IAppService& app_service;

    std::vector<CommandDescriptor> command_descriptors;
    std::vector<std::string> command_entries;
    int selected_command = 0;
    int committed_command = -1;

    std::string payload;
    bool executing = false;
    std::string executed_command;
    std::optional<CommandResult> command_result;
    std::mutex result_mutex;
    std::thread execute_thread;
    std::function<void()> on_update;

    bool validCommittedCommand() const;
    void executeCommandAsync(const UserCommand& cmd);
  };

  CliUiController::CliUiControllerImpl::CliUiControllerImpl(IAppService& appsrv) : app_service(appsrv)
  {
    command_descriptors = app_service.getCommandDescriptors();
    for (const CommandDescriptor& descriptor : command_descriptors) {
      command_entries.push_back(std::string(descriptor.name));
    }
  }

  bool CliUiController::CliUiControllerImpl::validCommittedCommand() const
  {
    return committed_command >= 0 && committed_command < static_cast<int>(command_descriptors.size());
  }

  void CliUiController::CliUiControllerImpl::executeCommandAsync(const UserCommand& cmd)
  {
    if (execute_thread.joinable()) {
      execute_thread.join();
    }

    execute_thread = std::thread([this, cmd]() {
      CommandResult result = app_service.executeCommand(cmd);
      {
        std::lock_guard<std::mutex> lock(result_mutex);
        command_result = std::move(result);
        executing = false;
      }
      if (on_update) {
        on_update();
      }
    });
  }

  CliUiController::CliUiController(IAppService& appsrv) : d(std::make_unique<CliUiControllerImpl>(appsrv))
  {
  }
  CliUiController::~CliUiController()
  {
    joinExecution();
  }
  CliUiController::CliUiController(CliUiController&&) noexcept = default;
  CliUiController& CliUiController::operator=(CliUiController&&) noexcept = default;

  const std::vector<CommandDescriptor>& CliUiController::commandDescriptors() const
  {
    return d->command_descriptors;
  }
  const std::vector<std::string>& CliUiController::commandEntries() const
  {
    return d->command_entries;
  }
  std::vector<std::string>& CliUiController::commandEntries()
  {
    return d->command_entries;
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

  void CliUiController::setOnUpdate(std::function<void()> callback)
  {
    d->on_update = std::move(callback);
  }

  void CliUiController::executeCurrentCommand()
  {
    if (!d->validCommittedCommand()) {
      return;
    }

    UserCommand cmd;
    cmd.cmd_type = d->command_descriptors[static_cast<std::size_t>(d->committed_command)].type;
    cmd.payload = d->payload;

    {
      std::lock_guard<std::mutex> lock(d->result_mutex);
      if (d->executing) {
        return;
      }
      d->executing = true;
      d->executed_command = std::string(d->command_descriptors[static_cast<std::size_t>(d->committed_command)].name);
    }

    d->executeCommandAsync(cmd);
  }

  void CliUiController::joinExecution()
  {
    if (d->execute_thread.joinable()) {
      d->execute_thread.join();
    }
  }

} // namespace pendarlab::app::mavlink_hub
