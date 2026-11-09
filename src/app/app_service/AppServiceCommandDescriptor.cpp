#include "app/app_service/AppServiceCommandDescriptor.h"

#include <array>
#include <cstddef>

namespace pendarlab::app::mavlink_hub
{
  namespace
  {
    static_assert(kAppServiceCommandCount == static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT_ALL) + 1,
                  "AppServiceCommandDescriptor table must have one entry per AppServiceCommandType enumerator");

    // The descriptor table is indexed by static_cast<std::size_t>(AppServiceCommandType),
    // so the first enumerator must have value 0 for the mapping to be correct.
    static_assert(static_cast<std::size_t>(AppServiceCommandType::LOAD_PLAN_FROM_PATH) == 0,
                  "AppServiceCommandType must start at 0 so it stays aligned with the kCommands descriptor table");

    // Canonical descriptor table for all commands, one entry per
    // AppServiceCommandType enumerator, in enum order. Adding a command requires
    // editing BOTH AppServiceCommandType (AppServiceCommandDescriptor.h) and this
    // table together, inserting the new command in the same position in both, and
    // keeping the first enumerator (LOAD_PLAN_FROM_PATH) at value 0, so the enum
    // and table stay aligned as the command set grows.
    constexpr std::array<AppServiceCommandDescriptor, kAppServiceCommandCount> kCommands = {
      {
       { AppServiceCommandType::LOAD_PLAN_FROM_PATH, "load_plan_from_path", true, "path to .json plan" },
       { AppServiceCommandType::LOAD_PLAN_FROM_JSON_TEXT, "load_plan_from_json_text", true, "plan JSON text" },
       { AppServiceCommandType::GET_CURRENT_PLAN, "get_current_plan", false, "" },
       { AppServiceCommandType::CHECK_CURRENT_PLAN, "check_current_plan", false, "" },
       { AppServiceCommandType::APPLY_CURRENT_PLAN, "apply_current_plan", false, "" },
       { AppServiceCommandType::DELETE_CURRENT_PLAN, "delete_current_plan", false, "" },
       { AppServiceCommandType::GET_STATUS, "get_status", false, "" },
       { AppServiceCommandType::ADD_MAVLINK_ENDPOINT, "add_mavlink_endpoint", true, "endpoint JSON" },
       { AppServiceCommandType::EDIT_MAVLINK_ENDPOINT, "edit_mavlink_endpoint", true, "endpoint JSON" },
       { AppServiceCommandType::CONNECT_MAVLINK_ENDPOINT, "connect_mavlink_endpoint", true, "endpoint name" },
       { AppServiceCommandType::DISCONNECT_MAVLINK_ENDPOINT, "disconnect_mavlink_endpoint", true, "endpoint name" },
       { AppServiceCommandType::REMOVE_MAVLINK_ENDPOINT, "remove_mavlink_endpoint", true, "endpoint name" },
       { AppServiceCommandType::GET_ENDPOINT_LIST, "get_endpoint_list", false, "" },
       { AppServiceCommandType::GET_ENDPOINT_STATUS, "get_endpoint_status", true, "endpoint name" },
       { AppServiceCommandType::GET_ENDPOINT_STATUS_ALL, "get_endpoint_status_all", false, "" },
       { AppServiceCommandType::ADD_AGENT, "add_agent", true, "agent JSON" },
       { AppServiceCommandType::EDIT_AGENT, "edit_agent", true, "agent JSON" },
       { AppServiceCommandType::START_AGENT, "start_agent", true, "agent name" },
       { AppServiceCommandType::STOP_AGENT, "stop_agent", true, "agent name" },
       { AppServiceCommandType::REMOVE_AGENT, "remove_agent", true, "agent name" },
       { AppServiceCommandType::GET_AGENT_LIST, "get_agent_list", false, "" },
       { AppServiceCommandType::GET_AGENT_STATUS, "get_agent_status", true, "agent name" },
       { AppServiceCommandType::GET_AGENT_STATUS_ALL, "get_agent_status_all", false, "" },
       { AppServiceCommandType::GET_AGENT_REPORT, "get_agent_report", true, "agent name" },
       { AppServiceCommandType::GET_AGENT_REPORT_ALL, "get_agent_report_all", false, "" },
       }
    };
  } // namespace

  const std::array<AppServiceCommandDescriptor, kAppServiceCommandCount>& appServiceCommandDescriptors()
  {
    return kCommands;
  }

} // namespace pendarlab::app::mavlink_hub
