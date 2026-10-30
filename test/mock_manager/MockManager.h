#pragma once

#include "common/types/OperationResult.h"
#include "manager/IManager.h"
#include "manager/types/ExecutionResultList.h"
#include "manager/types/UserPlan.h"

#include <mavlink_endpoint/MavlinkEndpointState.h>
#include <mavlink_hub_sdk/agent/AgentState.h>
#include <mavlink_hub_sdk/mavlink_endpoint_user/IMavlinkEndpointUser.h>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace pendarlab::app::mavlink_hub::test
{
  /// Minimal fake of IManager for AppService tests.
  ///
  /// Only the list/state query methods (getMavlinkEndpointList/getMavlinkEndpointState/
  /// getMavlinkEndpointStateAll, getAgentList/getAgentState/getAgentStateAll) are backed
  /// by settable data; every other method is a no-op returning a benign default. This keeps
  /// the fake small while exercising AppService::executeCommand's query commands.
  class MockManager : public IManager
  {
    using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
    using AgentState = pendarlab::sdk::mavlink_hub::AgentState;
    using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

  public:
    /// Data the query methods return. Tests populate these before calling executeCommand.
    std::vector<std::string> endpoint_list;
    std::unordered_map<std::string, MavlinkEndpointState> endpoint_states;
    std::vector<std::string> agent_list;
    std::unordered_map<std::string, AgentState> agent_states;

    virtual ExecutionResultList validatePlan(const UserPlan& plan) override
    {
      return ExecutionResultList{};
    }

    virtual ExecutionResultList executePlan(const UserPlan& plan) override
    {
      return ExecutionResultList{};
    }

    virtual std::vector<std::string> getMavlinkEndpointList() override
    {
      return endpoint_list;
    }

    virtual std::optional<MavlinkEndpointState> getMavlinkEndpointState(const std::string& name) override
    {
      auto it = endpoint_states.find(name);
      if (it == endpoint_states.end()) {
        return std::nullopt;
      }
      return it->second;
    }

    virtual std::unordered_map<std::string, MavlinkEndpointState> getMavlinkEndpointStateAll() override
    {
      return endpoint_states;
    }

    virtual std::vector<std::string> getAgentList() override
    {
      return agent_list;
    }

    virtual std::optional<AgentState> getAgentState(const std::string& name) override
    {
      auto it = agent_states.find(name);
      if (it == agent_states.end()) {
        return std::nullopt;
      }
      return it->second;
    }

    virtual std::unordered_map<std::string, AgentState> getAgentStateAll() override
    {
      return agent_states;
    }

    virtual OperationResult validateMavlinkEndpointConfig(const std::string& type,
                                                          const std::unordered_map<std::string, std::string>& config) override
    {
      return OperationResult{};
    }

    virtual OperationResult addMavlinkEndpoint(const std::string& name) override
    {
      return OperationResult{};
    }

    virtual std::unique_ptr<IMavlinkEndpointUser> createMavlinkEndpointUser(const std::string& endpoint_name,
                                                                            const std::string& requester_name) override
    {
      return nullptr;
    }

    virtual bool removeMavlinkEndpointUser(const std::string& endpoint_name, const std::string& requester_name) override
    {
      return true;
    }

    virtual std::optional<std::unordered_map<std::string, int>> getMavlinkEndpointUserList(const std::string& endpoint_name) override
    {
      return std::unordered_map<std::string, int>{};
    }

    virtual OperationResult connectMavlinkEndpoint(const std::string& name, const std::string& type,
                                                   const std::unordered_map<std::string, std::string>& config) override
    {
      return OperationResult{};
    }

    virtual OperationResult disconnectMavlinkEndpoint(const std::string& name) override
    {
      return OperationResult{};
    }

    virtual OperationResult removeMavlinkEndpoint(const std::string& name) override
    {
      return OperationResult{};
    }

    virtual OperationResult validateAgentConfig(const std::string& type,
                                                const std::unordered_map<std::string, std::string>& config) override
    {
      return OperationResult{};
    }

    virtual OperationResult addAgent(const std::string& name, const std::string& type,
                                     const std::unordered_map<std::string, std::string>& config) override
    {
      return OperationResult{};
    }

    virtual OperationResult editAgent(const std::string& name, const std::string& type,
                                      const std::unordered_map<std::string, std::string>& config) override
    {
      return OperationResult{};
    }

    virtual OperationResult startAgent(const std::string& name) override
    {
      return OperationResult{};
    }

    virtual OperationResult stopAgent(const std::string& name) override
    {
      return OperationResult{};
    }

    virtual OperationResult removeAgent(const std::string& name) override
    {
      return OperationResult{};
    }
  };

} // namespace pendarlab::app::mavlink_hub::test
