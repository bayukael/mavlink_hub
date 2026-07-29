#pragma once

#include "manager/IManager.h"

#include <mavlink_hub_sdk/manager_resource_requester/IManagerResourceRequester.h>
#include <mavlink_hub_sdk/mavlink_endpoint_user/IMavlinkEndpointUser.h>
#include <memory>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  class ManagerResourceRequester : public pendarlab::sdk::mavlink_hub::IManagerResourceRequester
  {
    using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;

  public:
    ManagerResourceRequester(const std::string& name, IManager* const mgr);
    ~ManagerResourceRequester();
    ManagerResourceRequester(ManagerResourceRequester&&) noexcept;
    ManagerResourceRequester& operator=(ManagerResourceRequester&&) noexcept;

    virtual std::unique_ptr<IMavlinkEndpointUser> requestMavlinkEndpoint(const std::string& ep_name) override;

  private:
    struct ManagerResourceRequesterImpl;
    std::unique_ptr<ManagerResourceRequesterImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub
