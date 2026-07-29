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

  class MavlinkEndpointUser : public pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser
  {
    using MavlinkEndpoint = pendarlab::lib::comm::MavlinkEndpoint;
    using MavlinkEndpointPacket = pendarlab::lib::comm::MavlinkEndpointPacket;
    using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;
    using MavlinkEndpointToken = pendarlab::lib::comm::MavlinkEndpointToken;

  public:
    MavlinkEndpointUser(IManager* mgr, std::weak_ptr<MavlinkEndpoint> ep, const std::string& ep_name, const std::string& user_name);
    ~MavlinkEndpointUser();
    MavlinkEndpointUser(MavlinkEndpointUser&&) noexcept;
    MavlinkEndpointUser& operator=(MavlinkEndpointUser&&) noexcept;

    virtual std::unique_ptr<MavlinkEndpointToken> createListener(std::function<void(const MavlinkEndpointPacket&)> listener_cb) override;
    virtual std::optional<int> getNumOfListener() override;
    virtual std::optional<int> writeMessage(const mavlink_message_t& msg) override;
    virtual std::optional<MavlinkEndpointState> getState() override;

  private:
    struct MavlinkEndpointUserImpl;
    std::unique_ptr<MavlinkEndpointUserImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub
