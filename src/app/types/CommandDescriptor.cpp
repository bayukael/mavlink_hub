#include "app/types/CommandDescriptor.h"

#include <array>
#include <cstddef>

namespace pendarlab::app::mavlink_hub
{
  namespace
  {
    constexpr std::size_t kCommandCount = 23;
    static_assert(kCommandCount == static_cast<std::size_t>(UserCommandType::GET_AGENT_STATUS_ALL) + 1,
                  "CommandDescriptor table must have one entry per UserCommandType enumerator");

    constexpr std::array<CommandDescriptor, kCommandCount> kCommands = {
      {
       { UserCommandType::LOAD_PLAN_FROM_PATH, "load_plan_from_path", true, "path to .json plan" },
       { UserCommandType::LOAD_PLAN_FROM_JSON_TEXT, "load_plan_from_json_text", true, "plan JSON text" },
       { UserCommandType::GET_CURRENT_PLAN, "get_current_plan", false, "" },
       { UserCommandType::CHECK_CURRENT_PLAN, "check_current_plan", false, "" },
       { UserCommandType::APPLY_CURRENT_PLAN, "apply_current_plan", false, "" },
       { UserCommandType::DELETE_CURRENT_PLAN, "delete_current_plan", false, "" },
       { UserCommandType::GET_STATUS, "get_status", false, "" },
       { UserCommandType::ADD_MAVLINK_ENDPOINT, "add_mavlink_endpoint", true, "endpoint JSON" },
       { UserCommandType::EDIT_MAVLINK_ENDPOINT, "edit_mavlink_endpoint", true, "endpoint JSON" },
       { UserCommandType::CONNECT_MAVLINK_ENDPOINT, "connect_mavlink_endpoint", true, "endpoint name" },
       { UserCommandType::DISCONNECT_MAVLINK_ENDPOINT, "disconnect_mavlink_endpoint", true, "endpoint name" },
       { UserCommandType::REMOVE_MAVLINK_ENDPOINT, "remove_mavlink_endpoint", true, "endpoint name" },
       { UserCommandType::GET_ENDPOINT_LIST, "get_endpoint_list", false, "" },
       { UserCommandType::GET_ENDPOINT_STATUS, "get_endpoint_status", true, "endpoint name" },
       { UserCommandType::GET_ENDPOINT_STATUS_ALL, "get_endpoint_status_all", false, "" },
       { UserCommandType::ADD_AGENT, "add_agent", true, "agent JSON" },
       { UserCommandType::EDIT_AGENT, "edit_agent", true, "agent JSON" },
       { UserCommandType::START_AGENT, "start_agent", true, "agent name" },
       { UserCommandType::STOP_AGENT, "stop_agent", true, "agent name" },
       { UserCommandType::REMOVE_AGENT, "remove_agent", true, "agent name" },
       { UserCommandType::GET_AGENT_LIST, "get_agent_list", false, "" },
       { UserCommandType::GET_AGENT_STATUS, "get_agent_status", true, "agent name" },
       { UserCommandType::GET_AGENT_STATUS_ALL, "get_agent_status_all", false, "" },
       }
    };
  } // namespace

  const std::vector<CommandDescriptor>& commandDescriptors()
  {
    static const std::vector<CommandDescriptor> descriptors(kCommands.begin(), kCommands.end());
    return descriptors;
  }

} // namespace pendarlab::app::mavlink_hub
