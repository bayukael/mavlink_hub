#pragma once

#include "app/lib_loader/LibInfo.h"

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  /// Startup configuration parsed from the app config file.
  struct AppConfig {
    std::unordered_map<std::string, LibInfo> agent_lib_list;     ///< Agent libraries to load at startup, keyed by registration name.
    std::unordered_map<std::string, LibInfo> transport_lib_list; ///< Transport libraries to load at startup, keyed by registration name.
    std::string path_to_extra_lib_list;                          ///< Optional path to a JSON file with additional libraries to load.
    std::string path_to_startup_user_plan;                       ///< Optional path to a user plan to load at startup.
    bool apply_user_plan_on_startup;                             ///< If true, apply the startup plan after loading it.
  };
} // namespace pendarlab::app::mavlink_hub
