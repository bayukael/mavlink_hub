#include "app/app_service/AppServiceCommandDescriptor.h"

#include <array>
#include <cstddef>

namespace pendarlab::app::mavlink_hub
{
  namespace
  {
    constexpr std::size_t kCommandCount = 25;
    static_assert(kCommandCount == static_cast<std::size_t>(AppServiceCommandType::GET_AGENT_REPORT_ALL) + 1,
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
    constexpr std::array<AppServiceCommandDescriptor, kCommandCount> kCommands = {
      {
       { AppServiceCommandType::LOAD_PLAN_FROM_PATH, "load_plan_from_path", true, "path to .json plan",
         "Load a plan from a .json file at the given path into the app service." },
       { AppServiceCommandType::LOAD_PLAN_FROM_JSON_TEXT, "load_plan_from_json_text", true, "plan JSON text",
         "Load a plan from raw plan JSON text provided as the payload." },
       { AppServiceCommandType::GET_CURRENT_PLAN, "get_current_plan", false, "",
         "Get the plan currently loaded in the app service." },
       { AppServiceCommandType::CHECK_CURRENT_PLAN, "check_current_plan", false, "",
         "Check the current plan for correctness or readiness." },
       { AppServiceCommandType::APPLY_CURRENT_PLAN, "apply_current_plan", false, "",
         "Apply the currently loaded plan to its target." },
       { AppServiceCommandType::DELETE_CURRENT_PLAN, "delete_current_plan", false, "",
         "Delete the currently loaded plan from the app service." },
       { AppServiceCommandType::GET_STATUS, "get_status", false, "",
         "Get the overall status of the app service." },
       { AppServiceCommandType::ADD_MAVLINK_ENDPOINT, "add_mavlink_endpoint", true, "endpoint JSON",
         "Add a new MAVLink endpoint from an endpoint definition in JSON." },
       { AppServiceCommandType::EDIT_MAVLINK_ENDPOINT, "edit_mavlink_endpoint", true, "endpoint JSON",
         "Edit an existing MAVLink endpoint from an updated endpoint definition in JSON." },
       { AppServiceCommandType::CONNECT_MAVLINK_ENDPOINT, "connect_mavlink_endpoint", true, "endpoint name",
         "Connect to the named MAVLink endpoint." },
       { AppServiceCommandType::DISCONNECT_MAVLINK_ENDPOINT, "disconnect_mavlink_endpoint", true, "endpoint name",
         "Disconnect from the named MAVLink endpoint." },
       { AppServiceCommandType::REMOVE_MAVLINK_ENDPOINT, "remove_mavlink_endpoint", true, "endpoint name",
         "Remove the named MAVLink endpoint from the app service." },
       { AppServiceCommandType::GET_ENDPOINT_LIST, "get_endpoint_list", false, "",
         "Get the list of known MAVLink endpoints." },
       { AppServiceCommandType::GET_ENDPOINT_STATUS, "get_endpoint_status", true, "endpoint name",
         "Get the status of the named MAVLink endpoint." },
       { AppServiceCommandType::GET_ENDPOINT_STATUS_ALL, "get_endpoint_status_all", false, "",
         "Get the status of all known MAVLink endpoints." },
       { AppServiceCommandType::ADD_AGENT, "add_agent", true, "agent JSON",
         "Add a new agent from an agent definition in JSON." },
       { AppServiceCommandType::EDIT_AGENT, "edit_agent", true, "agent JSON",
         "Edit an existing agent from an updated agent definition in JSON." },
       { AppServiceCommandType::START_AGENT, "start_agent", true, "agent name",
         "Start the named agent." },
       { AppServiceCommandType::STOP_AGENT, "stop_agent", true, "agent name",
         "Stop the named agent." },
       { AppServiceCommandType::REMOVE_AGENT, "remove_agent", true, "agent name",
         "Remove the named agent from the app service." },
       { AppServiceCommandType::GET_AGENT_LIST, "get_agent_list", false, "",
         "Get the list of known agents." },
       { AppServiceCommandType::GET_AGENT_STATUS, "get_agent_status", true, "agent name",
         "Get the status of the named agent." },
       { AppServiceCommandType::GET_AGENT_STATUS_ALL, "get_agent_status_all", false, "",
         "Get the status of all known agents." },
       { AppServiceCommandType::GET_AGENT_REPORT, "get_agent_report", true, "agent name",
         "Get the report of the named agent." },
       { AppServiceCommandType::GET_AGENT_REPORT_ALL, "get_agent_report_all", false, "",
         "Get the reports of all known agents." },
       }
    };
  } // namespace

  const std::vector<AppServiceCommandDescriptor>& appServiceCommandDescriptors()
  {
    static const std::vector<AppServiceCommandDescriptor> descriptors(kCommands.begin(), kCommands.end());
    return descriptors;
  }

} // namespace pendarlab::app::mavlink_hub
