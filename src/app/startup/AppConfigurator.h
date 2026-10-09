#pragma once

#include "app/app_service/IAppService.h"
#include "app/lib_loader/ILibLoader.h"
#include "app/types/AppConfig.h"
#include "common/types/OperationResult.h"

namespace pendarlab::app::mavlink_hub::startup
{
  /// Apply a startup AppConfig: load its agent/transport libraries, load an optional
  /// extra-library list file, and optionally load/apply a startup user plan.
  ///
  /// Libraries are loaded in order; failures are accumulated into the returned result.
  /// Processing stops early if an extra-library list file is missing or unparseable, or if a
  /// startup plan should be applied but fails to load.
  /// @param config the startup configuration to apply.
  /// @param lib_loader used to load the configured libraries.
  /// @param app_service used to issue the startup plan commands.
  /// @return success only if every step succeeded.
  OperationResult applyConfig(const AppConfig& config, ILibLoader& lib_loader, IAppService& app_service);
} // namespace pendarlab::app::mavlink_hub::startup
