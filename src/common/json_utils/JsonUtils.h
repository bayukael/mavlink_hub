#pragma once

#include "app/types/AppConfig.h"
#include "app/types/LibList.h"
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
  /// Parse a library list from a JSON array of {name, path, sym} objects.
  /// @return std::nullopt if the input is not an array, an entry is malformed, or a name repeats.
  std::optional<LibList> fstreamToLibList(std::ifstream& json_fstream);

  /// Parse an app config from a JSON file. Optional sections (lib lists, paths, plan flag)
  /// are only applied when present.
  /// @return std::nullopt on parse error or an invalid lib list.
  std::optional<AppConfig> fstreamToAppConfig(std::ifstream& json_fstream);

  /// Parse a user plan from a JSON file.
  /// @return std::nullopt on parse error or, under DISCARD policy, a malformed entry.
  std::optional<UserPlan> fstreamToUserPlan(std::ifstream& json_fstream);

  /// Parse a user plan from a JSON string (same semantics as fstreamToUserPlan).
  std::optional<UserPlan> stringToUserPlan(const std::string& json_str);

  /// Serialize an ExecutionResultList to a JSON string with "endpoint_plan_result" and
  /// "agent_plan_result" arrays of {name, success, messages} entries.
  std::string executionResultListToJsonString(const ExecutionResultList& list);

  /// Serialize an agent name list to a JSON string under the "agent_list" key.
  std::string agentListToJsonString(const std::vector<std::string>& list);

  /// Serialize a single agent state to a JSON string under the "agent_status" key.
  std::string agentStatusToJsonString(const pendarlab::sdk::mavlink_hub::AgentState& state);

  /// Serialize a name->agent-state map to a JSON string under the "agent_status_list" key.
  std::string agentStatusListToJsonString(const std::unordered_map<std::string, pendarlab::sdk::mavlink_hub::AgentState>& list);

  /// Serialize an endpoint name list to a JSON string under the "endpoint_list" key.
  std::string endpointListToJsonString(const std::vector<std::string>& list);

  /// Serialize a single endpoint state to a JSON string under the "endpoint_status" key.
  std::string endpointStatusToJsonString(const pendarlab::lib::comm::MavlinkEndpointState& state);

  /// Serialize a name->endpoint-state map to a JSON string under the "endpoint_status_list" key.
  std::string endpointStatusListToJsonString(const std::unordered_map<std::string, pendarlab::lib::comm::MavlinkEndpointState>& list);

} // namespace pendarlab::app::mavlink_hub::json_utils