#pragma once

#include "app/app_service/IAppService.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"

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
  };

  inline CommandResult MockAppService::executeCommand(const UserCommand& cmd)
  {
    commands.push_back(cmd);

    CommandResult result;
    result.success = true;
    switch (cmd.cmd_type) {
      case UserCommandType::LOAD_PLAN_FROM_PATH: result.success = !fail_load_plan; break;
      case UserCommandType::APPLY_CURRENT_PLAN: result.success = !fail_apply_plan; break;
      default: break;
    }
    result.message.push_back(result.success ? "[MockAppService]: ok" : "[MockAppService]: failed");
    return result;
  }
} // namespace pendarlab::app::mavlink_hub::test
