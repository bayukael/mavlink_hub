#pragma once

#include "manager/IManager.h"

#include <mavlink_endpoint/MavlinkEndpoint.h>
#include <mavlink_endpoint/MavlinkEndpointPacket.h>
#include <mavlink_endpoint/MavlinkEndpointState.h>
#include <mavlink_endpoint/MavlinkEndpointToken.h>
#include <mavlink_hub_sdk/mavlink_endpoint_user/IMavlinkEndpointUser.h>
#include <memory>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  class Manager;

  /// A per-requester handle to a MAVLink endpoint.
  ///
  /// Created by IManager::createMavlinkEndpointUser. Wraps a weak reference to the underlying
  /// endpoint; operations return std::nullopt/nullptr if the endpoint has been removed.
  /// Destruction releases one reference held by the requester on the endpoint (see
  /// IManager::removeMavlinkEndpointUser).
  class MavlinkEndpointUser : public pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser
  {
    using MavlinkEndpoint = pendarlab::lib::comm::MavlinkEndpoint;
    using MavlinkEndpointPacket = pendarlab::lib::comm::MavlinkEndpointPacket;
    using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
    using MavlinkEndpointToken = pendarlab::lib::comm::MavlinkEndpointToken;

  public:
    /// @param mgr the manager used to release the user reference on destruction.
    /// @param ep weak handle to the underlying endpoint.
    /// @param ep_name name of the endpoint, for reference accounting.
    /// @param user_name identity of the requester, for reference accounting.
    MavlinkEndpointUser(IManager* mgr, std::weak_ptr<MavlinkEndpoint> ep, const std::string& ep_name, const std::string& user_name);
    ~MavlinkEndpointUser();
    MavlinkEndpointUser(MavlinkEndpointUser&&) noexcept;
    MavlinkEndpointUser& operator=(MavlinkEndpointUser&&) noexcept;

    /// Subscribe \p listener_cb to incoming packets on the endpoint.
    /// @return a token that unsubscribes the listener when destroyed, or nullptr if the
    ///         endpoint is gone.
    virtual std::unique_ptr<MavlinkEndpointToken> createListener(std::function<void(const MavlinkEndpointPacket&)> listener_cb) override;

    /// @return the number of current listeners, or std::nullopt if the endpoint is gone.
    virtual std::optional<int> getNumOfListener() override;

    /// Send \p msg on the endpoint.
    /// @return a status/result of the write, or std::nullopt if the endpoint is gone.
    virtual std::optional<int> writeMessage(const mavlink_message_t& msg) override;

    /// @return the current endpoint state, or std::nullopt if the endpoint is gone.
    virtual std::optional<MavlinkEndpointState> getState() override;

  private:
    struct MavlinkEndpointUserImpl;
    std::unique_ptr<MavlinkEndpointUserImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub
