#pragma once

#include "app/types/StartupIntent.h"

namespace pendarlab::app::mavlink_hub::startup
{
  StartupIntent parseArgs(int argc, char** argv);
} // namespace pendarlab::app::mavlink_hub::startup
