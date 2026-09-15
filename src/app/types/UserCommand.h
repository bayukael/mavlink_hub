#pragma once

#include "app/types/CommandDescriptor.h"

#include <string>

namespace pendarlab::app::mavlink_hub
{
  struct UserCommand {
    UserCommandType cmd_type;
    std::string payload;
  };

} // namespace pendarlab::app::mavlink_hub
