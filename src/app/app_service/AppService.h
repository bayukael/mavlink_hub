#pragma once

#include "app/lib_loader/LibLoader.h"
#include "app/types/CommandResult.h"
#include "app/types/UserCommand.h"
#include "manager/IManager.h"

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  class AppService
  {
  public:
    AppService(IManager& mgr, LibLoader& lib_loader);
    ~AppService();
    AppService(AppService&&) noexcept;
    AppService& operator=(AppService&&) noexcept;

    virtual CommandResult executeCommand(const UserCommand& cmd);

  private:
    struct AppServiceImpl;
    std::unique_ptr<AppServiceImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
