#pragma once

#include "app/app_service/IAppService.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"

#include <chrono>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace pendarlab::app::mavlink_hub::test
{
  class MockAppService : public IAppService
  {
  public:
    CommandResult executeCommand(const UserCommand& cmd) override;

    std::vector<UserCommand> commands;
    bool fail_load_plan = false;
    bool fail_apply_plan = false;

    // Status data surfaced for the startup overview population fetch. Keyed by
    // entity name; the value is the raw AgentState / MavlinkEndpointState enum
    // integer, matching what the real AppService/JsonUtils status-list encoders
    // produce. Empty by default so unrelated tests see an empty overview.
    std::unordered_map<std::string, int> agent_status_all;
    std::unordered_map<std::string, int> endpoint_status_all;

    // Optional: make a specific command sleep before returning, to give a
    // window for testing async / ignored-execute behavior. Default (no
    // slow_command, or zero delay) leaves existing behavior unchanged.
    std::optional<UserCommandType> slow_command;
    std::chrono::milliseconds slow_delay{};

  private:
    static std::string encodeStatusList(const char* key, const std::unordered_map<std::string, int>& statuses);
  };

  inline std::string MockAppService::encodeStatusList(const char* key, const std::unordered_map<std::string, int>& statuses)
  {
    std::string out = std::string("{ \"") + key + "\": [";
    bool first = true;
    for (const auto& [name, status] : statuses) {
      if (!first) {
        out += ", ";
      }
      first = false;
      out += "{ \"name\": \"" + name + "\", \"status\": " + std::to_string(status) + " }";
    }
    out += " ] }";
    return out;
  }

  inline CommandResult MockAppService::executeCommand(const UserCommand& cmd)
  {
    commands.push_back(cmd);

    if (slow_command.has_value() && cmd.cmd_type == *slow_command && slow_delay.count() > 0) {
      std::this_thread::sleep_for(slow_delay);
    }

    CommandResult result;
    result.success = true;
    switch (cmd.cmd_type) {
      case UserCommandType::LOAD_PLAN_FROM_PATH: result.success = !fail_load_plan; break;
      case UserCommandType::APPLY_CURRENT_PLAN: result.success = !fail_apply_plan; break;
      case UserCommandType::GET_AGENT_STATUS_ALL:
        result.data = encodeStatusList("agent_status_list", agent_status_all);
        break;
      case UserCommandType::GET_ENDPOINT_STATUS_ALL:
        result.data = encodeStatusList("endpoint_status_list", endpoint_status_all);
        break;
      default: break;
    }
    result.message.push_back(result.success ? "[MockAppService]: ok" : "[MockAppService]: failed");
    return result;
  }
} // namespace pendarlab::app::mavlink_hub::test
