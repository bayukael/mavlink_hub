#pragma once

#include "common/types/OperationResult.h"
#include "manager/types/ExecutionResultList.h"
#include "manager/types/UserPlan.h"

#include <mavlink_endpoint/MavlinkEndpointState.h>
#include <mavlink_hub_sdk/agent/AgentState.h>
#include <mavlink_hub_sdk/mavlink_endpoint_user/IMavlinkEndpointUser.h>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  class IManager
  {
    using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
    using AgentState = pendarlab::sdk::mavlink_hub::AgentState;
    using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

  public:
    virtual ~IManager() = default;
    virtual ExecutionResultList executePlan(const UserPlan& plan) = 0;
    virtual ExecutionResultList validatePlan(const UserPlan& plan) = 0;
    virtual std::vector<std::string> getMavlinkEndpointList() = 0;
    virtual std::optional<MavlinkEndpointState> getMavlinkEndpointState(const std::string& name) = 0;
    virtual std::unordered_map<std::string, MavlinkEndpointState> getMavlinkEndpointStateAll() = 0;
    virtual std::vector<std::string> getAgentList() = 0;
    virtual std::optional<AgentState> getAgentState(const std::string& name) = 0;
    virtual std::unordered_map<std::string, AgentState> getAgentStateAll() = 0;

    virtual OperationResult validateMavlinkEndpointConfig(const std::string& type,
                                                          const std::unordered_map<std::string, std::string>& config) = 0;
    virtual OperationResult addMavlinkEndpoint(const std::string& name) = 0;
    virtual std::unique_ptr<IMavlinkEndpointUser> createMavlinkEndpointUser(const std::string& endpoint_name,
                                                                            const std::string& requester_name) = 0;
    virtual bool removeMavlinkEndpointUser(const std::string& endpoint_name, const std::string& requester_name) = 0;
    virtual std::optional<std::unordered_map<std::string, int>> getMavlinkEndpointUserList(const std::string& endpoint_name) = 0;
    virtual OperationResult connectMavlinkEndpoint(const std::string& name, const std::string& type,
                                                   const std::unordered_map<std::string, std::string>& config) = 0;
    virtual OperationResult disconnectMavlinkEndpoint(const std::string& name) = 0;
    virtual OperationResult removeMavlinkEndpoint(const std::string& name) = 0;

    virtual OperationResult validateAgentConfig(const std::string& type, const std::unordered_map<std::string, std::string>& config) = 0;
    virtual OperationResult addAgent(const std::string& name, const std::string& type,
                                     const std::unordered_map<std::string, std::string>& config) = 0;
    virtual OperationResult editAgent(const std::string& name, const std::string& type,
                                      const std::unordered_map<std::string, std::string>& config) = 0;
    virtual OperationResult startAgent(const std::string& name) = 0;
    virtual OperationResult stopAgent(const std::string& name) = 0;
    virtual OperationResult removeAgent(const std::string& name) = 0;
  };

} // namespace pendarlab::app::mavlink_hub
