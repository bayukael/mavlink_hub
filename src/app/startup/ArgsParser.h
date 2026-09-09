#pragma once

#include "app/startup/Intent.h"
#include "app/types/AppConfig.h"

#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub::startup
{
  std::optional<AppConfig> parseArgs(int argc, char** argv);
} // namespace pendarlab::app::mavlink_hub::startup
