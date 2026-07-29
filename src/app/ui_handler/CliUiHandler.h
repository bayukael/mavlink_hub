#pragma once

#include "app/app_service/AppService.h"
#include "common/types/OperationResult.h"

#include <memory>

namespace pendarlab::app::mavlink_hub
{
  class CliUiHandler
  {
  public:
    CliUiHandler(AppService& appsrv);
    ~CliUiHandler();
    CliUiHandler(CliUiHandler&&) noexcept;
    CliUiHandler& operator=(CliUiHandler&&) noexcept;

    void start();
    void stop();
    OperationResult getResult();

  private:
    struct CliUiHandlerImpl;
    std::unique_ptr<CliUiHandlerImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
