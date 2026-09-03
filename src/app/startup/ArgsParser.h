#pragma once

#include "app/types/StartupIntent.h"

#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub::startup
{
  std::optional<StartupIntent> parseArgs(int argc, char** argv);
} // namespace pendarlab::app::mavlink_hub::startup
