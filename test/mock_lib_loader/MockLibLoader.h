#pragma once

#include "app/lib_loader/ILibLoader.h"
#include "app/lib_loader/LibInfo.h"
#include "common/types/OperationResult.h"

#include <vector>

namespace pendarlab::app::mavlink_hub::test
{
  class MockLibLoader : public ILibLoader
  {
  public:
    OperationResult loadAgentLib(const LibInfo& lib_info) override;
    OperationResult loadTransportLib(const LibInfo& lib_info) override;

    std::vector<LibInfo> loaded_agent_libs;
    std::vector<LibInfo> loaded_transport_libs;
    bool fail_agent_lib = false;
    bool fail_transport_lib = false;
  };

  inline OperationResult MockLibLoader::loadAgentLib(const LibInfo& lib_info)
  {
    OperationResult result;
    if (fail_agent_lib) {
      result.success = false;
      result.messages.push_back("[MockLibLoader::loadAgentLib]: failed");
      return result;
    }
    loaded_agent_libs.push_back(lib_info);
    result.success = true;
    return result;
  }

  inline OperationResult MockLibLoader::loadTransportLib(const LibInfo& lib_info)
  {
    OperationResult result;
    if (fail_transport_lib) {
      result.success = false;
      result.messages.push_back("[MockLibLoader::loadTransportLib]: failed");
      return result;
    }
    loaded_transport_libs.push_back(lib_info);
    result.success = true;
    return result;
  }
} // namespace pendarlab::app::mavlink_hub::test
