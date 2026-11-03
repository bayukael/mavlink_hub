#include "app/app_service/AppService.h"

#include "app/lib_loader/ILibLoader.h"
#include "app/types/CommandResult.h"
#include "app/app_service/CommandDescriptor.h"
#include "common/json_utils/JsonUtils.h"

#include <fstream>
#include <optional>

namespace pendarlab::app::mavlink_hub
{
  struct AppService::AppServiceImpl {
    AppServiceImpl(IManager&, ILibLoader&);
    IManager& manager;
    ILibLoader& lib_loader;
    std::optional<UserPlan> current_plan;
  };

  AppService::AppServiceImpl::AppServiceImpl(IManager& mgr, ILibLoader& loader) : manager(mgr), lib_loader(loader)
  {
  }

  AppService::AppService(IManager& mgr, ILibLoader& loader) : d(std::make_unique<AppServiceImpl>(mgr, loader))
  {
  }
  AppService::~AppService()
  {
  }
  AppService::AppService(AppService&&) noexcept = default;
  AppService& AppService::operator=(AppService&&) noexcept = default;

  CommandResult AppService::executeCommand(const AppServiceCommand& cmd)
  {
    CommandResult result;
    switch (cmd.cmd_type) {
      case AppServiceCommandType::LOAD_PLAN_FROM_PATH: {
        std::ifstream file_stream(cmd.payload);
        if (file_stream.is_open()) {
          std::optional<UserPlan> user_plan = json_utils::fstreamToUserPlan(file_stream);
          if (user_plan.has_value()) {
            d->current_plan = user_plan;
            result.success = true;
            result.message.push_back("[AppService]: Plan is loaded");
          } else {
            result.success = false;
            result.message.push_back("[AppService]: Plan is not loaded because of parsing error");
          }
        } else {
          result.success = false;
          result.message.push_back("[AppService]: Plan is not loaded because the file [" + cmd.payload + "] does not exist");
        }
        break;
      }

      case AppServiceCommandType::LOAD_PLAN_FROM_JSON_TEXT: {
        std::optional<UserPlan> user_plan = json_utils::stringToUserPlan(cmd.payload);
        if (user_plan.has_value()) {
          d->current_plan = user_plan;
          result.success = true;
          result.message.push_back("[AppService]: Plan is loaded");
        } else {
          result.success = false;
          result.message.push_back("[AppService]: Plan is not loaded because of parsing error");
        }
        break;
      }

      case AppServiceCommandType::GET_CURRENT_PLAN:
        /* code */
        break;

      case AppServiceCommandType::CHECK_CURRENT_PLAN: {
        if (d->current_plan.has_value()) {
          auto execute_result = d->manager.validatePlan(d->current_plan.value());
          result.success = true;
          result.message.push_back("[AppService]: The current plan has been checked and the detailed report has been produced.");
          result.data = json_utils::executionResultListToJsonString(execute_result);
        } else {
          result.success = false;
          result.message.push_back("[AppService]: Plan is not checked because there is no plan loaded");
        }
        break;
      }

      case AppServiceCommandType::APPLY_CURRENT_PLAN: {
        if (d->current_plan.has_value()) {
          auto execute_result = d->manager.executePlan(d->current_plan.value());
          result.success = true;
          result.message.push_back("[AppService]: The current plan is applied and the detailed report has been produced.");
          result.data = json_utils::executionResultListToJsonString(execute_result);
        } else {
          result.success = false;
          result.message.push_back("[AppService]: Plan is not applied because there is no plan loaded");
        }
        break;
      }

      case AppServiceCommandType::DELETE_CURRENT_PLAN: {
        d->current_plan = std::nullopt;
        result.success = true;
        result.message.push_back("[AppService]: Current plan is removed. There is no loaded plan now.");
        break;
      }

      case AppServiceCommandType::GET_STATUS:
        /* code */
        break;

      case AppServiceCommandType::ADD_MAVLINK_ENDPOINT:
        /* code */
        break;

      case AppServiceCommandType::EDIT_MAVLINK_ENDPOINT:
        /* code */
        break;

      case AppServiceCommandType::CONNECT_MAVLINK_ENDPOINT:
        /* code */
        break;

      case AppServiceCommandType::DISCONNECT_MAVLINK_ENDPOINT:
        /* code */
        break;

      case AppServiceCommandType::REMOVE_MAVLINK_ENDPOINT:
        /* code */
        break;

      case AppServiceCommandType::GET_ENDPOINT_LIST: {
        result.success = true;
        result.message.push_back("[AppService]: Endpoint list has been retrieved.");
        result.data = json_utils::endpointListToJsonString(d->manager.getMavlinkEndpointList());
        break;
      }

      case AppServiceCommandType::GET_ENDPOINT_STATUS: {
        if (cmd.payload.empty()) {
          result.success = false;
          result.message.push_back("[AppService]: Endpoint status is not retrieved because the endpoint name is missing");
          break;
        }
        auto endpoint_state = d->manager.getMavlinkEndpointState(cmd.payload);
        if (!endpoint_state.has_value()) {
          result.success = false;
          result.message.push_back("[AppService]: Endpoint status is not retrieved because the endpoint name [" + cmd.payload + "] is not registered");
          break;
        }
        result.success = true;
        result.message.push_back("[AppService]: Endpoint status has been retrieved.");
        result.data = json_utils::endpointStatusToJsonString(endpoint_state.value());
        break;
      }

      case AppServiceCommandType::GET_ENDPOINT_STATUS_ALL: {
        result.success = true;
        result.message.push_back("[AppService]: Endpoint statuses have been retrieved.");
        result.data = json_utils::endpointStatusListToJsonString(d->manager.getMavlinkEndpointStateAll());
        break;
      }

      case AppServiceCommandType::ADD_AGENT:
        /* code */
        break;

      case AppServiceCommandType::EDIT_AGENT:
        /* code */
        break;

      case AppServiceCommandType::START_AGENT:
        /* code */
        break;

      case AppServiceCommandType::STOP_AGENT:
        /* code */
        break;

      case AppServiceCommandType::REMOVE_AGENT:
        /* code */
        break;

      case AppServiceCommandType::GET_AGENT_LIST: {
        result.success = true;
        result.message.push_back("[AppService]: Agent list has been retrieved.");
        result.data = json_utils::agentListToJsonString(d->manager.getAgentList());
        break;
      }

      case AppServiceCommandType::GET_AGENT_STATUS: {
        if (cmd.payload.empty()) {
          result.success = false;
          result.message.push_back("[AppService]: Agent status is not retrieved because the agent name is missing");
          break;
        }
        auto agent_state = d->manager.getAgentState(cmd.payload);
        if (!agent_state.has_value()) {
          result.success = false;
          result.message.push_back("[AppService]: Agent status is not retrieved because the agent name [" + cmd.payload + "] is not registered");
          break;
        }
        result.success = true;
        result.message.push_back("[AppService]: Agent status has been retrieved.");
        result.data = json_utils::agentStatusToJsonString(agent_state.value());
        break;
      }

      case AppServiceCommandType::GET_AGENT_STATUS_ALL: {
        result.success = true;
        result.message.push_back("[AppService]: Agent statuses have been retrieved.");
        result.data = json_utils::agentStatusListToJsonString(d->manager.getAgentStateAll());
        break;
      }

      case AppServiceCommandType::GET_AGENT_REPORT: {
        if (cmd.payload.empty()) {
          result.success = false;
          result.message.push_back("[AppService]: Agent report is not retrieved because the agent name is missing");
          break;
        }
        auto agent_report = d->manager.getAgentReport(cmd.payload);
        if (!agent_report.has_value()) {
          result.success = false;
          result.message.push_back("[AppService]: Agent report is not retrieved because the agent name [" + cmd.payload + "] is not registered");
          break;
        }
        result.success = true;
        result.message.push_back("[AppService]: Agent report has been retrieved.");
        result.data = json_utils::agentReportToJsonString(cmd.payload, agent_report.value());
        break;
      }

      case AppServiceCommandType::GET_AGENT_REPORT_ALL: {
        result.success = true;
        result.message.push_back("[AppService]: Agent reports have been retrieved.");
        result.data = json_utils::agentReportListToJsonString(d->manager.getAgentReportAll());
        break;
      }

      default: break;
    }
    return result;
  }

} // namespace pendarlab::app::mavlink_hub