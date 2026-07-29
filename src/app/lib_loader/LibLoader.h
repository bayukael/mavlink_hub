#pragma once

#include "agent_registry/AgentRegistryAdminAccess.h"
#include "common/types/OperationResult.h"

#include <byte_transport/RegistryAdminAccess.h>
#include <memory>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  class LibLoader
  {
    using TransportRegistryAdminAccess = pendarlab::lib::comm::byte_transport::RegistryAdminAccess;

  public:
    LibLoader(AgentRegistryAdminAccess&, TransportRegistryAdminAccess&);
    ~LibLoader();
    LibLoader(LibLoader&&) noexcept;
    LibLoader& operator=(LibLoader&&) noexcept;

    OperationResult loadAgentLib(const std::string& agent_def_name, const std::string& lib_path, const std::string& sym);
    OperationResult loadTransportLib(const std::string& transport_def_name, const std::string& lib_path, const std::string& sym);

  private:
    struct LibLoaderImpl;
    std::unique_ptr<LibLoaderImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub