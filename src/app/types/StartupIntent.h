#pragma once

#include "app/types/AppConfig.h"
#include "manager/types/UserPlan.h"

#include <optional>

namespace pendarlab::app::mavlink_hub
{
  struct StartupIntent {
    std::string config_file_path;
  };
} // namespace pendarlab::app::mavlink_hub
