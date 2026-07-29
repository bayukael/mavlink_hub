#pragma once

#include <string>

namespace pendarlab::app::mavlink_hub
{
  struct AppConfig {
    std::string agent_libs_dir_path;
    std::string transport_libs_dir_path;
  };
} // namespace pendarlab::app::mavlink_hub
