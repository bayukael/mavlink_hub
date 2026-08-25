#pragma once

#include "manager/types/UserPlan.h"
#include "manager/types/ExecutionResultList.h"

#include <fstream>
#include <optional>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  std::optional<UserPlan> fstreamToUserPlan(std::ifstream& json_fstream);
  std::optional<UserPlan> stringToUserPlan(const std::string& json_str);
  std::string executionResultListToString(const ExecutionResultList& list);
} // namespace pendarlab::app::mavlink_hub