#include "app/app_service/CommandDescriptor.h"

#include <array>
#include <cstddef>

namespace pendarlab::app::mavlink_hub
{
  namespace
  {
    constexpr std::size_t kCommandCount = 25;
    static_assert(kCommandCount == static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT_ALL) + 1,
                  "CommandDescriptor table must have one entry per AppServiceCommandType enumerator");

    constexpr std::array<CommandDescriptor, kCommandCount> kCommands = {
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

  const std::vector<CommandDescriptor>& appServiceCommandDescriptors()
  {
    static const std::vector<CommandDescriptor> descriptors(kCommands.begin(), kCommands.end());
    return descriptors;
  }

} // namespace pendarlab::app::mavlink_hub
