#pragma once

#include "app/app_service/IAppService.h"
#include "app/lib_loader/ILibLoader.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"
#include "manager/IManager.h"

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  /// Concrete IAppService delegating to an IManager and ILibLoader.
  ///
  /// Tracks the currently loaded user plan (from LOAD_PLAN_FROM_PATH / LOAD_PLAN_FROM_JSON_TEXT)
  /// and supports plan lifecycle commands. Commands that require a currently loaded plan fail
  /// with a clear message when none is present.
  class AppService : public IAppService
  {
  public:
    AppService(IManager& mgr, ILibLoader& lib_loader);
    ~AppService();
    AppService(AppService&&) noexcept;
    AppService& operator=(AppService&&) noexcept;

    CommandResult executeCommand(const UserCommand& cmd) override;

  private:
    struct AppServiceImpl;
    std::unique_ptr<AppServiceImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
