#pragma once

#include "app/types/StartupIntent.h"

#include <optional>
#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub::startup
{
  std::unordered_map<std::string, std::string> parseArgs(int argc, char** argv);
} // namespace pendarlab::app::mavlink_hub::startup
