#pragma once

#include "app/lib_loader/LibInfo.h"
#include "common/types/OperationResult.h"

namespace pendarlab::app::mavlink_hub
{
  class ILibLoader
  {
  public:
    virtual ~ILibLoader() = default;

    virtual OperationResult loadAgentLib(const LibInfo& lib_info) = 0;
    virtual OperationResult loadTransportLib(const LibInfo& lib_info) = 0;
  };
} // namespace pendarlab::app::mavlink_hub
