#pragma once

#include "app/lib_loader/LibInfo.h"

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  /// Collection of libraries keyed by registration name.
  typedef std::unordered_map<std::string, LibInfo> LibList;
} // namespace pendarlab::app::mavlink_hub
