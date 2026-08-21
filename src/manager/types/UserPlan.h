#pragma once

#include "manager/types/AgentEntry.h"
#include "manager/types/MavlinkEndpointEntry.h"
#include "manager/types/UserPlanPolicy.h"

#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub
{
  struct UserPlan {
    std::unordered_map<std::string, MavlinkEndpointEntry> endpoint_list;
    std::unordered_map<std::string, AgentEntry> agent_list;
    UserPlanPolicy policy;
  };
} // namespace pendarlab::app::mavlink_hub
