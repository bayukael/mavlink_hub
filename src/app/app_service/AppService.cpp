#include "app/app_service/AppService.h"

#include "app/lib_loader/LibLoader.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommandType.h"
#include "common/json_utils/JsonUtils.h"
#include "manager/Manager.h"

#include <fstream>
#include <optional>

namespace pendarlab::app::mavlink_hub
{
  struct AppService::AppServiceImpl {
    AppServiceImpl(IManager&, LibLoader&);
    IManager& manager;
    LibLoader& lib_loader;
    std::optional<UserPlan> current_plan;
  };

  AppService::AppServiceImpl::AppServiceImpl(IManager& mgr, LibLoader& loader) : manager(mgr), lib_loader(loader)
  {
  }

  AppService::AppService(IManager& mgr, LibLoader& loader) : d(std::make_unique<AppServiceImpl>(mgr, loader))
  {
  }
  AppService::~AppService()
  {
  }
  AppService::AppService(AppService&&) noexcept = default;
  AppService& AppService::operator=(AppService&&) noexcept = default;

  CommandResult AppService::executeCommand(const UserCommand& cmd)
  {
    CommandResult result;
    switch (cmd.cmd_type) {
      case UserCommandType::LOAD_PLAN_FROM_PATH: {
        std::ifstream file_stream(cmd.payload);
        if (file_stream.is_open()) {
          std::optional<UserPlan> user_plan = fstreamToUserPlan(file_stream);
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

      case UserCommandType::LOAD_PLAN_FROM_JSON_TEXT: {
        std::optional<UserPlan> user_plan = stringToUserPlan(cmd.payload);
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

      case UserCommandType::GET_CURRENT_PLAN:
        /* code */
        break;

      case UserCommandType::APPLY_CURRENT_PLAN: {
        if (d->current_plan.has_value()) {
          auto execute_result = d->manager.executePlan(d->current_plan.value());
          result.success = true;
          result.message.push_back("[AppService]: The current plan is applied and the detailed report has been produced.");
          result.data = executionResultListToString(execute_result);
        } else {
          result.success = false;
          result.message.push_back("[AppService]: Plan is not applied because there is no plan loaded");
        }
        break;
      }

      case UserCommandType::DELETE_CURRENT_PLAN: {
        d->current_plan = std::nullopt;
        result.success = true;
        result.message.push_back("[AppService]: Current plan is removed. There is no loaded plan now.");
        break;
      }

      case UserCommandType::GET_STATUS:
        /* code */
        break;

      case UserCommandType::ADD_MAVLINK_ENDPOINT:
        /* code */
        break;

      case UserCommandType::EDIT_MAVLINK_ENDPOINT:
        /* code */
        break;

      case UserCommandType::CONNECT_MAVLINK_ENDPOINT:
        /* code */
        break;

      case UserCommandType::DISCONNECT_MAVLINK_ENDPOINT:
        /* code */
        break;

      case UserCommandType::REMOVE_MAVLINK_ENDPOINT:
        /* code */
        break;

      case UserCommandType::GET_ENDPOINT_LIST:
        /* code */
        break;

      case UserCommandType::GET_ENDPOINT_STATUS:
        /* code */
        break;

      case UserCommandType::GET_ENDPOINT_STATUS_ALL:
        /* code */
        break;

      case UserCommandType::ADD_AGENT:
        /* code */
        break;

      case UserCommandType::EDIT_AGENT:
        /* code */
        break;

      case UserCommandType::START_AGENT:
        /* code */
        break;

      case UserCommandType::STOP_AGENT:
        /* code */
        break;

      case UserCommandType::REMOVE_AGENT:
        /* code */
        break;

      case UserCommandType::GET_AGENT_LIST:
        /* code */
        break;

      case UserCommandType::GET_AGENT_STATUS:
        /* code */
        break;

      case UserCommandType::GET_AGENT_STATUS_ALL:
        /* code */
        break;

      default: break;
    }
    return result;
  }

} // namespace pendarlab::app::mavlink_hub