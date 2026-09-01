#include "common/json_utils/JsonUtils.h"

#include "manager/types/MavlinkEndpointEntry.h"

#include <jsoncons/json.hpp>

using namespace jsoncons;
using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
using AgentState = pendarlab::sdk::mavlink_hub::AgentState;

namespace pendarlab::app::mavlink_hub
{
  std::optional<std::unordered_map<std::string, MavlinkEndpointEntry>> jsonToEndpointList(const json& j, const UserPlanPolicy& policy)
  {
    // TODO: We should consider a different return value where it can show us what errors happen in parsing.
    if (!j.is_array()) { // j should be an array. If not, we don't proceed
      return std::nullopt;
    }

    std::unordered_map<std::string, MavlinkEndpointEntry> endpoint_list;
    for (const auto& entry : j.array_range()) {
      if (!entry.contains("name") || !entry.contains("type") || !entry.contains("config") || !entry.contains("connect_on_create")) {
        if (policy == UserPlanPolicy::BEST_EFFORT) {
          continue;
        } else {
          return std::nullopt;
        }
      }

      if (!entry["config"].is_object()) {
        if (policy == UserPlanPolicy::BEST_EFFORT) {
          continue;
        } else {
          return std::nullopt;
        }
      }

      if (!entry["connect_on_create"].is_bool()) {
        if (policy == UserPlanPolicy::BEST_EFFORT) {
          continue;
        } else {
          return std::nullopt;
        }
      }

      std::unordered_map<std::string, std::string> config;
      for (const auto& config_entry : entry["config"].object_range()) {
        config[config_entry.key()] = config_entry.value().as_string();
      }

      const std::string& name = entry["name"].as_string();
      const std::string& type = entry["type"].as_string();
      const bool& connect_on_create = entry["connect_on_create"].as_bool();
      MavlinkEndpointEntry endpoint_entry;
      endpoint_entry.type = type;
      endpoint_entry.config = config;
      endpoint_entry.connect_on_create = connect_on_create;
      endpoint_list[name] = endpoint_entry;
    }
    return endpoint_list;
  }

  std::optional<std::unordered_map<std::string, AgentEntry>> jsonToAgentList(const json& j, const UserPlanPolicy& policy)
  {
    // TODO: We should consider a different return value where it can show us what errors happen in parsing.
    if (!j.is_array()) { // j should be an array. If not, we don't proceed
      return std::nullopt;
    }

    std::unordered_map<std::string, AgentEntry> agent_list;
    for (const auto& entry : j.array_range()) {
      if (!entry.contains("name") || !entry.contains("type") || !entry.contains("config")) {
        if (policy == UserPlanPolicy::BEST_EFFORT) {
          continue;
        } else {
          return std::nullopt;
        }
      }

      if (!entry["config"].is_object()) {
        if (policy == UserPlanPolicy::BEST_EFFORT) {
          continue;
        } else {
          return std::nullopt;
        }
      }

      std::unordered_map<std::string, std::string> config;
      for (const auto& config_entry : entry["config"].object_range()) {
        config[config_entry.key()] = config_entry.value().as_string();
      }

      const std::string& name = entry["name"].as_string();
      const std::string& type = entry["type"].as_string();
      AgentEntry agent_entry;
      agent_entry.type = type;
      agent_entry.config = config;
      agent_list[name] = agent_entry;
    }
    return agent_list;
  }

  std::optional<UserPlan> jsonToUserPlan(const json& plan_json)
  {
    UserPlan plan_result;

    // Extracting policy
    if (plan_json.contains("policy")) {
      std::string user_plan_policy = plan_json["policy"].as_string();
      if (user_plan_policy == "discard") {
        plan_result.policy = UserPlanPolicy::DISCARD;
      } else if (user_plan_policy == "best_effort") {
        plan_result.policy = UserPlanPolicy::BEST_EFFORT;
      } else { // unknown policy, return nullopt
        return std::nullopt;
      }
    }

    // Extracting Endpoint List
    // If it does not contain endpoint_list, we assume endpoint_list is empty
    if (plan_json.contains("endpoint_list")) {
      auto endpoint_list = jsonToEndpointList(plan_json["endpoint_list"], plan_result.policy);
      if (!endpoint_list.has_value()) {
        if (plan_result.policy == UserPlanPolicy::DISCARD) {
          return std::nullopt;
        } else { // For best effort policy, we assume the list as an empty list.
          endpoint_list = std::unordered_map<std::string, MavlinkEndpointEntry>();
        }
      }
      plan_result.endpoint_list = endpoint_list.value();
    }

    // Extracting Agent List
    // If it does not contain agent_list, we assume agent_list is empty
    if (plan_json.contains("agent_list")) {
      auto agent_list = jsonToAgentList(plan_json["agent_list"], plan_result.policy);
      if (!agent_list.has_value()) {
        if (plan_result.policy == UserPlanPolicy::DISCARD) {
          return std::nullopt;
        } else { // For best effort policy, we assume the list as an empty list.
          agent_list = std::unordered_map<std::string, AgentEntry>();
        }
      }
      plan_result.agent_list = agent_list.value();
    }
    return plan_result;
  }

  std::string agentStateToString(const AgentState& state)
  {
    std::string result;
    switch (state) {
      case AgentState::ACTIVE: {
        result = "active";
        break;
      }

      case AgentState::IDLE: {
        result = "idle";
        break;
      }

      case AgentState::STARTING: {
        result = "starting";
        break;
      }

      case AgentState::STOPPING: {
        result = "stopping";
        break;
      }

      default: {
        result = "unknown agent state";
        break;
      }
    }
    return result;
  }

  std::string endpointStateToString(const MavlinkEndpointState& state)
  {
    std::string result;
    switch (state) {
      case MavlinkEndpointState::CONNECTED: {
        result = "connected";
        break;
      }

      case MavlinkEndpointState::CONNECTING: {
        result = "connecting";
        break;
      }

      case MavlinkEndpointState::DISCONNECTED: {
        result = "disconnected";
        break;
      }

      case MavlinkEndpointState::DISCONNECTING: {
        result = "disconnecting";
        break;
      }

      default: {
        result = "unknown endpoint state";
        break;
      }
    }
    return result;
  }

  //--------------------------------------------------------------------------------------------------------------------------------------------

  std::optional<UserPlan> fstreamToUserPlan(std::ifstream& json_fstream)
  {
    json plan_json;
    try {
      plan_json = json::parse(json_fstream);
    } catch (const ser_error& e) {
      return std::nullopt;
    }
    return jsonToUserPlan(plan_json);
  }

  std::optional<UserPlan> stringToUserPlan(const std::string& json_str)
  {
    json plan_json;
    try {
      plan_json = json::parse(json_str);
    } catch (const ser_error& e) {
      return std::nullopt;
    }
    return jsonToUserPlan(plan_json);
  }

  std::string executionResultListToJsonString(const ExecutionResultList& list)
  {
    json execution_result;

    execution_result["endpoint_plan_result"] = json{ json_array_arg };
    for (const auto& [name, result] : list.endpoint_plan_result) {
      json entry_result;
      entry_result["name"] = name;
      entry_result["success"] = result.success;
      entry_result["messages"] = json{ json_array_arg };
      for (const auto& msg : result.messages) {
        entry_result["messages"].push_back(msg);
      }
      execution_result["endpoint_plan_result"].push_back(entry_result);
    }

    execution_result["agent_plan_result"] = json{ json_array_arg };
    for (const auto& [name, result] : list.agent_plan_result) {
      json entry_result;
      entry_result["name"] = name;
      entry_result["success"] = result.success;
      entry_result["messages"] = json{ json_array_arg };
      for (const auto& msg : result.messages) {
        entry_result["messages"].push_back(msg);
      }
      execution_result["agent_plan_result"].push_back(entry_result);
    }

    return execution_result.as_string();
  }

  json vectorOfStringToJson(const std::vector<std::string>& list)
  {
    json j_vec{ json_array_arg };
    for (const auto& str : list) {
      j_vec.push_back(str);
    }
    return j_vec;
  }

  std::string agentListToJsonString(const std::vector<std::string>& list)
  {
    json j_agent_list;
    j_agent_list["agent_list"] = vectorOfStringToJson(list);
    return j_agent_list.as_string();
  }

  std::string endpointListToJsonString(const std::vector<std::string>& list)
  {
    json j_endpoint_list;
    j_endpoint_list["endpoint_list"] = vectorOfStringToJson(list);
    return j_endpoint_list.as_string();
  }

  std::string agentStatusToJsonString(const pendarlab::sdk::mavlink_hub::AgentState& state)
  {
    json j_res;
    j_res["agent_status"] = agentStateToString(state);
    return j_res.as_string();
  }

  std::string endpointStatusToJsonString(const pendarlab::lib::comm::MavlinkEndpointState& state)
  {
    json j_res;
    j_res["endpoint_status"] = endpointStateToString(state);
    return j_res.as_string();
  }

  std::string agentStatusListToJsonString(const std::unordered_map<std::string, pendarlab::sdk::mavlink_hub::AgentState>& list)
  {
    json j_res;
    j_res["agent_status_list"] = json{json_array_arg};
    for (const auto& [name, status]: list){
      json j_entry;
      j_entry["name"] = name;
      j_entry["status"] = status;
      j_res["agent_status_list"].push_back(j_entry);
    }
    return j_res.as_string();
  }

  std::string endpointStatusListToJsonString(const std::unordered_map<std::string, pendarlab::lib::comm::MavlinkEndpointState>& list)
  {
    json j_res;
    j_res["endpoint_status_list"] = json{json_array_arg};
    for (const auto& [name, status]: list){
      json j_entry;
      j_entry["name"] = name;
      j_entry["status"] = status;
      j_res["endpoint_status_list"].push_back(j_entry);
    }
    return j_res.as_string();
  }
} // namespace pendarlab::app::mavlink_hub