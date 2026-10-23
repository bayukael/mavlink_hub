#include "manager/ManagerResourceRequester.h"

#include "manager/IManager.h"

#include <string>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  using IMavlinkEndpointUser = pendarlab::sdk::mavlink_hub::IMavlinkEndpointUser;
  struct ManagerResourceRequester::ManagerResourceRequesterImpl {
    ManagerResourceRequesterImpl(const std::string& name, IManager* const mgr);
    const std::string name;
    IManager* const manager;
  };

  ManagerResourceRequester::ManagerResourceRequesterImpl::ManagerResourceRequesterImpl(const std::string& n, IManager* const mgr) :
      name(n), manager(mgr)
  {
  }

  ManagerResourceRequester::ManagerResourceRequester(const std::string& n, IManager* const mgr) :
      d(std::make_unique<ManagerResourceRequesterImpl>(n, mgr))
  {
  }

  ManagerResourceRequester::~ManagerResourceRequester()
  {
  }

  ManagerResourceRequester::ManagerResourceRequester(ManagerResourceRequester&&) noexcept = default;
  ManagerResourceRequester& ManagerResourceRequester::operator=(ManagerResourceRequester&&) noexcept = default;

  std::unique_ptr<IMavlinkEndpointUser> ManagerResourceRequester::requestMavlinkEndpoint(const std::string& ep_name)
  {
    return d->manager->createMavlinkEndpointUser(ep_name, d->name);
  }

  std::vector<std::string> ManagerResourceRequester::getMavlinkEndpointList()
  {
    return d->manager->getMavlinkEndpointList();
  }

  std::string ManagerResourceRequester::getName(){
    return d->name;
  }

} // namespace pendarlab::app::mavlink_hub