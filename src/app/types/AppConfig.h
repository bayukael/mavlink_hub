#pragma once

#include "app/lib_loader/LibInfo.h"

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  struct AppConfig {
    std::unordered_map<std::string, LibInfo> agent_lib_list;
    std::unordered_map<std::string, LibInfo> transport_lib_list;
    std::string path_to_extra_lib_list;
    std::string path_to_startup_user_plan;
  };
} // namespace pendarlab::app::mavlink_hub
