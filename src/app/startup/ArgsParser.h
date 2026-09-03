#pragma once

#include "app/startup/Intent.h"

#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub::startup
{
  std::optional<Intent> parseArgs(int argc, char** argv);
} // namespace pendarlab::app::mavlink_hub::startup
