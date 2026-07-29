#pragma once

#include "common/types/OperationResult.h"

#include <unordered_map>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  struct ExecutionResultList {
    std::unordered_map<std::string, OperationResult> endpoint_plan_result;
    std::unordered_map<std::string, OperationResult> agent_plan_result;
  };
} // namespace pendarlab::app::mavlink_hub
