#pragma once

#include "agent_registry/AgentRegistryAdminAccess.h"
#include "app/lib_loader/ILibLoader.h"
#include "app/lib_loader/LibInfo.h"
#include "common/types/OperationResult.h"

#include <byte_transport/RegistryAdminAccess.h>
#include <memory>
#include <string>

namespace pendarlab::app::mavlink_hub
{
  /// ILibLoader implementation using POSIX dlopen/dlsym.
  ///
  /// Opens each library with RTLD_NOW | RTLD_GLOBAL, resolves the given symbol, and expects
  /// it to be a function returning a pointer to a definition (AgentDefinition or
  /// TransportDefinition). The definition is registered in the corresponding admin registry
  /// and the library handle is kept open until this object is destroyed, at which point all
  /// handles are closed.
  class LibLoader : public ILibLoader
  {
    using TransportRegistryAdminAccess = pendarlab::lib::comm::byte_transport::RegistryAdminAccess;

  public:
    /// @param agent_registry destination for loaded agent definitions.
    /// @param transport_registry destination for loaded transport definitions.
    LibLoader(AgentRegistryAdminAccess&, TransportRegistryAdminAccess&);
    ~LibLoader();
    LibLoader(LibLoader&&) noexcept;
    LibLoader& operator=(LibLoader&&) noexcept;

    /// Load an agent library, registering the definition under \p agent_def_name.
    /// @return success only if the library loads and the resolved symbol yields a valid
    ///         AgentDefinition that registers.
    OperationResult loadAgentLib(const std::string& agent_def_name, const std::string& lib_path, const std::string& sym);

    /// Load a transport library, registering the definition under \p transport_def_name.
    /// @return success only if the library loads and the resolved symbol yields a valid
    ///         TransportDefinition that registers.
    OperationResult loadTransportLib(const std::string& transport_def_name, const std::string& lib_path, const std::string& sym);

    OperationResult loadAgentLib(const LibInfo& lib_info) override;
    OperationResult loadTransportLib(const LibInfo& lib_info) override;

  private:
    struct LibLoaderImpl;
    std::unique_ptr<LibLoaderImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub