#pragma once

#include "agent_registry/AgentRegistryUserAccess.h"
#include "common/types/OperationResult.h"
#include "manager/IManager.h"
#include "manager/types/ExecutionResultList.h"
#include "manager/types/UserPlan.h"

#include <byte_transport/RegistryUserAccess.h>
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
  /// Default IManager implementation.
  ///
  /// Holds the authoritative registries of endpoints and agents. Endpoint definitions and
  /// their transports come from the transport registry; agent definitions come from the
  /// agent registry. Both are supplied as read-only user access views, so the manager can
  /// look up registered types and their config parsers without mutating them.
  ///
  /// The class owns its state through a PIMPL handle; it is move-only and not copyable.
  /// Endpoints/agents are keyed by name, and each endpoint tracks per-requester reference
  /// counts so the same requester may hold several MavlinkEndpointUser handles.
  class Manager : public IManager
  {
    using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
    using AgentState = pendarlab::sdk::mavlink_hub::AgentState;
    using TransportRegistryUserAccess = pendarlab::lib::comm::byte_transport::RegistryUserAccess;
    using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

  public:
    /// @param agent_registry read-only view of the registered agent definitions.
    /// @param transport_registry read-only view of the registered transport definitions.
    Manager(const AgentRegistryUserAccess&, const TransportRegistryUserAccess&);
    virtual ~Manager();
    Manager(Manager&&) noexcept;
    Manager& operator=(Manager&&) noexcept;

    virtual ExecutionResultList executePlan(const UserPlan& plan) override;
    virtual ExecutionResultList validatePlan(const UserPlan& plan) override;
    virtual std::vector<std::string> getMavlinkEndpointList() override;
    virtual std::optional<MavlinkEndpointState> getMavlinkEndpointState(const std::string& name) override;
    virtual std::unordered_map<std::string, MavlinkEndpointState> getMavlinkEndpointStateAll() override;
    virtual std::vector<std::string> getAgentList() override;
    virtual std::optional<AgentState> getAgentState(const std::string& name) override;
    virtual std::unordered_map<std::string, AgentState> getAgentStateAll() override;

    virtual OperationResult validateMavlinkEndpointConfig(const std::string& type,
                                                          const std::unordered_map<std::string, std::string>& config) override;
    virtual OperationResult addMavlinkEndpoint(const std::string& name) override;
    virtual std::unique_ptr<IMavlinkEndpointUser> createMavlinkEndpointUser(const std::string& endpoint_name,
                                                                            const std::string& requester_name) override;
    virtual bool removeMavlinkEndpointUser(const std::string& endpoint_name, const std::string& requester_name) override;
    virtual std::optional<std::unordered_map<std::string, int>> getMavlinkEndpointUserList(const std::string& endpoint_name) override;
    virtual OperationResult connectMavlinkEndpoint(const std::string& name, const std::string& type,
                                                   const std::unordered_map<std::string, std::string>& config) override;
    virtual OperationResult disconnectMavlinkEndpoint(const std::string& name) override;
    virtual OperationResult removeMavlinkEndpoint(const std::string& name) override;

    virtual OperationResult validateAgentConfig(const std::string& type,
                                                const std::unordered_map<std::string, std::string>& config) override;
    virtual OperationResult addAgent(const std::string& name, const std::string& type,
                                     const std::unordered_map<std::string, std::string>& config) override;
    virtual OperationResult editAgent(const std::string& name, const std::string& type,
                                      const std::unordered_map<std::string, std::string>& config) override;
    virtual OperationResult startAgent(const std::string& name) override;
    virtual OperationResult stopAgent(const std::string& name) override;
    virtual OperationResult removeAgent(const std::string& name) override;

  private:
    struct ManagerImpl;
    std::unique_ptr<ManagerImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
