#pragma once

#include "common/types/OperationResult.h"

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  /// Per-entry outcome of validating or executing a UserPlan, keyed by endpoint/agent name.
  struct ExecutionResultList {
    std::unordered_map<std::string, OperationResult> endpoint_plan_result;
    std::unordered_map<std::string, OperationResult> agent_plan_result;
  };
} // namespace pendarlab::app::mavlink_hub
