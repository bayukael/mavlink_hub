#pragma once

#include "manager/types/ExecutionResultList.h"
#include "manager/types/UserPlan.h"

#include <fstream>
#include <mavlink_endpoint/MavlinkEndpointState.h>
#include <mavlink_hub_sdk/agent/AgentState.h>
#include <optional>
#include <string>
#include <unordered_map>

namespace pendarlab::app::mavlink_hub::json_utils
{
  std::optional<UserPlan> fstreamToUserPlan(std::ifstream& json_fstream);
  std::optional<UserPlan> stringToUserPlan(const std::string& json_str);
  std::string executionResultListToJsonString(const ExecutionResultList& list);
  std::string agentListToJsonString(const std::vector<std::string>& list);
  std::string agentStatusToJsonString(const pendarlab::sdk::mavlink_hub::AgentState& state);
  std::string agentStatusListToJsonString(const std::unordered_map<std::string, pendarlab::sdk::mavlink_hub::AgentState>& list);
  std::string endpointListToJsonString(const std::vector<std::string>& list);
  std::string endpointStatusToJsonString(const pendarlab::lib::comm::MavlinkEndpointState& state);
  std::string endpointStatusListToJsonString(const std::unordered_map<std::string, pendarlab::lib::comm::MavlinkEndpointState>& list);

} // namespace pendarlab::app::mavlink_hub::json_utils