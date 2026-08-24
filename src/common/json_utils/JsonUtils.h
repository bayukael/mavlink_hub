#pragma once

#include "manager/types/UserPlan.h"

#include <fstream>
#include <jsoncons/json.hpp>
#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  std::optional<UserPlan> fstreamToUserPlan(std::ifstream& json_fstream);
  std::optional<UserPlan> stringToUserPlan(const std::string& json_str);
} // namespace pendarlab::app::mavlink_hub