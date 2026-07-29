#pragma once

#include "app/types/AppConfig.h"
#include "manager/types/UserPlan.h"

#include <optional>

namespace pendarlab::app::mavlink_hub
{
  struct StartupIntent {
    AppConfig app_config;
    std::optional<UserPlan> plan;
    bool apply_plan_on_load = false;
  };
} // namespace pendarlab::app::mavlink_hub
