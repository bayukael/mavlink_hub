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
  /// Central management interface for MAVLink endpoints and agents.
  ///
  /// A manager owns the set of known MAVLink endpoints and agents and mediates all
  /// lifecycle operations on them (create, configure, connect/disconnect, start/stop,
  /// remove). It also evaluates user plans: a plan describes a desired set of
  /// endpoints and agents; the manager validates it and, when executed, makes the
  /// running system match it.
  ///
  /// Implementations are not guaranteed to be thread-safe.
  class IManager
  {
    using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
    using AgentState = pendarlab::sdk::mavlink_hub::AgentState;
    using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

  public:
    virtual ~IManager() = default;

    /// Validate every endpoint and agent entry in \p plan without changing manager state.
    ///
    /// Each entry is checked for name availability (collision with an already-registered
    /// endpoint/agent) and for a valid, registered \c type with a parseable \c config.
    /// The returned results are keyed by the entry name.
    ///
    /// @param plan the plan whose entries are to be validated.
    /// @return per-entry validation results; \c success is true only if both the name is
    ///         available and the type/config are valid.
    virtual ExecutionResultList validatePlan(const UserPlan& plan) = 0;

    /// Bring the manager into the state described by \p plan.
    ///
    /// The plan is validated first. The \c plan.policy then decides what happens to
    /// entries that fail validation:
    ///   - UserPlanPolicy::DISCARD: if any entry is invalid, nothing is executed;
    ///     every entry is reported as discarded.
    ///   - UserPlanPolicy::BEST_EFFORT: valid entries are applied while invalid entries
    ///     are skipped and reported as failed.
    /// Adding an endpoint with \c connect_on_create set also connects it.
    ///
    /// @param plan the desired target state.
    /// @return per-entry execution results (whether each endpoint/agent was created).
    /// @see validatePlan
    virtual ExecutionResultList executePlan(const UserPlan& plan) = 0;

    /// List the names of all known MAVLink endpoints.
    virtual std::vector<std::string> getMavlinkEndpointList() = 0;

    /// @return the current state of the named endpoint, or std::nullopt if it is not registered.
    virtual std::optional<MavlinkEndpointState> getMavlinkEndpointState(const std::string& name) = 0;

    /// @return the current state of every registered endpoint, keyed by endpoint name.
    virtual std::unordered_map<std::string, MavlinkEndpointState> getMavlinkEndpointStateAll() = 0;

    /// List the names of all registered agents.
    virtual std::vector<std::string> getAgentList() = 0;

    /// @return the current state of the named agent, or std::nullopt if it is not registered.
    virtual std::optional<AgentState> getAgentState(const std::string& name) = 0;

    /// @return the current state of every registered agent, keyed by agent name.
    virtual std::unordered_map<std::string, AgentState> getAgentStateAll() = 0;

    /// Validate an endpoint \c type against \p config without registering anything.
    ///
    /// @return success only if \p type is a registered transport and \p config parses
    ///         for it; failure messages are appended on error.
    virtual OperationResult validateMavlinkEndpointConfig(const std::string& type,
                                                          const std::unordered_map<std::string, std::string>& config) = 0;

    /// Register a new MAVlink endpoint by name. The endpoint is not connected yet.
    ///
    /// @return failure if an endpoint with \p name already exists.
    virtual OperationResult addMavlinkEndpoint(const std::string& name) = 0;

    /// Create a handle for \p requester_name to use \p endpoint_name.
    ///
    /// The returned handle can send/receive messages on the endpoint. Each call bumps the
    /// requester's reference count on that endpoint; the count is released when the handle
    /// is destroyed (see MavlinkEndpointUser).
    ///
    /// @return a new user handle, or nullptr if \p endpoint_name is not registered.
    /// @see removeMavlinkEndpointUser, getMavlinkEndpointUserList
    virtual std::unique_ptr<IMavlinkEndpointUser> createMavlinkEndpointUser(const std::string& endpoint_name,
                                                                            const std::string& requester_name) = 0;

    /// Release one reference held by \p requester_name on \p endpoint_name.
    ///
    /// @return true on success (including when there was nothing to release).
    /// @note A requester that holds multiple references must call this once per reference.
    virtual bool removeMavlinkEndpointUser(const std::string& endpoint_name, const std::string& requester_name) = 0;

    /// @return the per-requester reference counts for \p endpoint_name, or std::nullopt if
    ///         the endpoint is not registered.
    virtual std::optional<std::unordered_map<std::string, int>> getMavlinkEndpointUserList(const std::string& endpoint_name) = 0;

    /// Connect an existing endpoint to a transport.
    ///
    /// @return failure if the endpoint does not exist or \p type/\p config cannot connect.
    virtual OperationResult connectMavlinkEndpoint(const std::string& name, const std::string& type,
                                                   const std::unordered_map<std::string, std::string>& config) = 0;

    /// Disconnect an existing endpoint from its transport.
    ///
    /// @return failure if the endpoint does not exist or could not be disconnected.
    virtual OperationResult disconnectMavlinkEndpoint(const std::string& name) = 0;

    /// Unregister an endpoint, removing it and all its user references.
    ///
    /// @return failure if the endpoint does not exist.
    virtual OperationResult removeMavlinkEndpoint(const std::string& name) = 0;

    /// Validate an agent \c type against \p config without creating an agent.
    ///
    /// @return success only if \p type is a registered agent and \p config parses for it.
    virtual OperationResult validateAgentConfig(const std::string& type, const std::unordered_map<std::string, std::string>& config) = 0;

    /// Create and register a new agent.
    ///
    /// @return failure if the name is taken, the type is not registered, or the config
    ///         cannot be parsed/created.
    virtual OperationResult addAgent(const std::string& name, const std::string& type,
                                     const std::unordered_map<std::string, std::string>& config) = 0;

    /// Reconfigure an existing agent. Does not restart a running agent.
    ///
    /// @return failure if the agent does not exist, the type is not registered, or the
    ///         new config is rejected by the agent.
    virtual OperationResult editAgent(const std::string& name, const std::string& type,
                                      const std::unordered_map<std::string, std::string>& config) = 0;

    /// Start an existing agent.
    ///
    /// @return failure if the agent does not exist or its current state is not IDLE.
    virtual OperationResult startAgent(const std::string& name) = 0;

    /// Stop an existing agent.
    ///
    /// @return failure if the agent does not exist or is currently STARTING.
    virtual OperationResult stopAgent(const std::string& name) = 0;

    /// Unregister an agent.
    ///
    /// @return failure if the agent does not exist. The reported message includes the
    ///         agent's last state.
    virtual OperationResult removeAgent(const std::string& name) = 0;
  };

} // namespace pendarlab::app::mavlink_hub
