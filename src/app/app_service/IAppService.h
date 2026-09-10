#pragma once

#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"

namespace pendarlab::app::mavlink_hub
{
  class IAppService
  {
  public:
    virtual ~IAppService() = default;

    virtual CommandResult executeCommand(const UserCommand& cmd) = 0;
  };
} // namespace pendarlab::app::mavlink_hub
