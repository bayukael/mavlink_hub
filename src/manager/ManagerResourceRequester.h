#pragma once

#include "manager/IManager.h"

#include <mavlink_hub_sdk/manager_resource_requester/IManagerResourceRequester.h>
#include <mavlink_hub_sdk/mavlink_endpoint_user/IMavlinkEndpointUser.h>
#include <memory>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  /// A per-agent bridge that lets an agent ask the manager for MAVLink endpoint access.
  ///
  /// Each registered agent is given a ManagerResourceRequester carrying the agent's name.
  /// When the agent calls requestMavlinkEndpoint, the request is forwarded to the manager on
  /// the agent's behalf, so the endpoint's reference count is attributed to that agent.
  class ManagerResourceRequester : public pendarlab::sdk::mavlink_hub::IManagerResourceRequester
  {
    using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

  public:
    /// @param name identity of the requester (usually the owning agent's name), used when
    ///        attributing endpoint user references.
    /// @param mgr the manager to forward endpoint requests to. Must outlive this object.
    ManagerResourceRequester(const std::string& name, IManager* const mgr);
    ~ManagerResourceRequester();
    ManagerResourceRequester(ManagerResourceRequester&&) noexcept;
    ManagerResourceRequester& operator=(ManagerResourceRequester&&) noexcept;

    /// @return a handle to the requested endpoint, or nullptr if \p ep_name is not registered.
    /// @see IManager::createMavlinkEndpointUser
    virtual std::unique_ptr<IMavlinkEndpointUser> requestMavlinkEndpoint(const std::string& ep_name) override;

    /// @return the requester identity this instance was constructed with.
    std::string getName();

  private:
    struct ManagerResourceRequesterImpl;
    std::unique_ptr<ManagerResourceRequesterImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub
