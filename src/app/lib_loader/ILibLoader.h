#pragma once

#include "app/lib_loader/LibInfo.h"
#include "common/types/OperationResult.h"

namespace pendarlab::app::mavlink_hub
{
  /// Loads plugin libraries (agent and transport definitions) into the running process.
  ///
  /// Implementations open a shared library, resolve an exported symbol that returns a
  /// definition object, and register that definition under a name for later use by the
  /// manager.
  class ILibLoader
  {
  public:
    virtual ~ILibLoader() = default;

    /// Load and register an agent-definition library.
    /// @param lib_info identifies the library, symbol, and registration name.
    /// @return success only if the library loads and the definition registers.
    virtual OperationResult loadAgentLib(const LibInfo& lib_info) = 0;

    /// Load and register a transport-definition library.
    /// @param lib_info identifies the library, symbol, and registration name.
    /// @return success only if the library loads and the definition registers.
    virtual OperationResult loadTransportLib(const LibInfo& lib_info) = 0;
  };
} // namespace pendarlab::app::mavlink_hub
