#pragma once

#include "app/startup/Intent.h"
#include "app/types/AppConfig.h"

#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub::startup
{
  struct ParseResult {
    enum class Status {
      Ok,
      ShowHelp,
      FatalError
    };
    Status status = Status::Ok;
    int exit_code = 0;
    std::optional<AppConfig> config;
  };

  ParseResult parseArgs(int argc, char** argv);
} // namespace pendarlab::app::mavlink_hub::startup
