#include "app/app_service/AppService.h"

#include "app/lib_loader/LibLoader.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommandType.h"
#include "manager/Manager.h"

namespace pendarlab::app::mavlink_hub
{
  struct AppService::AppServiceImpl {
    AppServiceImpl(IManager&, LibLoader&);
    IManager& manager;
    LibLoader& lib_loader;
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
      case UserCommandType::LOAD_PLAN_FROM_PATH:
        /* code */
        break;

      case UserCommandType::LOAD_PLAN_FROM_JSON_TEXT:
        /* code */
        break;

      case UserCommandType::GET_CURRENT_PLAN:
        /* code */
        break;

      case UserCommandType::APPLY_CURRENT_PLAN:
        /* code */
        break;

      case UserCommandType::DELETE_CURRENT_PLAN:
        /* code */
        break;

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