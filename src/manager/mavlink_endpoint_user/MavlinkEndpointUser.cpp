#include "manager/mavlink_endpoint_user/MavlinkEndpointUser.h"

#include "manager/IManager.h"

#include <mavlink/common/mavlink.h>
#include <memory>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  using MavlinkEndpoint = pendarlab::lib::comm::MavlinkEndpoint;
  using MavlinkEndpointToken = pendarlab::lib::comm::MavlinkEndpointToken;
  using MavlinkEndpointState = pendarlab::lib::comm::MavlinkEndpointState;

  struct MavlinkEndpointUser::MavlinkEndpointUserImpl {
    MavlinkEndpointUserImpl(IManager* mgr, std::weak_ptr<MavlinkEndpoint>, const std::string& ep_name, const std::string& user_name);
    ~MavlinkEndpointUserImpl();
    IManager* manager;
    const std::string user_name;
    const std::string endpoint_name;
    std::weak_ptr<MavlinkEndpoint> endpoint;
  };

  MavlinkEndpointUser::MavlinkEndpointUserImpl::MavlinkEndpointUserImpl(IManager* mgr, std::weak_ptr<MavlinkEndpoint> ep,
                                                                        const std::string& ep_name, const std::string& user) :
      manager(mgr), endpoint(ep), endpoint_name(ep_name), user_name(user)
  {
  }

  MavlinkEndpointUser::MavlinkEndpointUserImpl::~MavlinkEndpointUserImpl()
  {
    manager->removeMavlinkEndpointUser(endpoint_name, user_name);
  }

  MavlinkEndpointUser::MavlinkEndpointUser(IManager* mgr, std::weak_ptr<MavlinkEndpoint> ep, const std::string& ep_name,
                                           const std::string& user_name) :
      d(std::make_unique<MavlinkEndpointUserImpl>(mgr, ep, ep_name, user_name))
  {
  }

  MavlinkEndpointUser::~MavlinkEndpointUser()
  {
  }

  MavlinkEndpointUser::MavlinkEndpointUser(MavlinkEndpointUser&&) noexcept = default;
  MavlinkEndpointUser& MavlinkEndpointUser::operator=(MavlinkEndpointUser&&) noexcept = default;

  std::unique_ptr<MavlinkEndpointToken> MavlinkEndpointUser::createListener(std::function<void(const MavlinkEndpointPacket&)> listener_cb)
  {
    if (std::shared_ptr<MavlinkEndpoint> ep = d->endpoint.lock()) {
      return ep->createListener(listener_cb);
    }

    return nullptr;
  }

  std::optional<int> MavlinkEndpointUser::getNumOfListener()
  {
    if (std::shared_ptr<MavlinkEndpoint> ep = d->endpoint.lock()) {
      return ep->getNumOfListener();
    }

    return std::nullopt;
  }

  std::optional<int> MavlinkEndpointUser::writeMessage(const mavlink_message_t& msg)
  {
    if (std::shared_ptr<MavlinkEndpoint> ep = d->endpoint.lock()) {
      return ep->writeMessage(msg);
    }

    return std::nullopt;
  }

  std::optional<MavlinkEndpointState> MavlinkEndpointUser::getState()
  {
    if (std::shared_ptr<MavlinkEndpoint> ep = d->endpoint.lock()) {
      return ep->getState();
    }

    return std::nullopt;
  }
} // namespace pendarlab::app::mavlink_hub