#pragma once

#include "app/app_service/IAppService.h"
#include "app/lib_loader/ILibLoader.h"
#include "app/types/AppConfig.h"
#include "common/types/OperationResult.h"

namespace pendarlab::app::mavlink_hub::startup
{
  OperationResult applyConfig(const AppConfig& config, ILibLoader& lib_loader, IAppService& app_service);
} // namespace pendarlab::app::mavlink_hub::startup
