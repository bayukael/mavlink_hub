#pragma once

#include "app/startup/Intent.h"

#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub::startup
{
  std::optional<Intent> parseArgs(int argc, char** argv, Intent default_intent);
} // namespace pendarlab::app::mavlink_hub::startup
