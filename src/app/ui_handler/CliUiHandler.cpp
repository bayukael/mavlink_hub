#include "app/ui_handler/CliUiHandler.h"
#include "common/types/OperationResult.h"

#include <memory>

// TODO: start a thread to run the handler

namespace pendarlab::app::mavlink_hub
{
  struct CliUiHandler::CliUiHandlerImpl {
    CliUiHandlerImpl(AppService& appsrv);
    AppService& app_service;
    OperationResult run_result;
  };

  CliUiHandler::CliUiHandlerImpl::CliUiHandlerImpl(AppService& appsrv) : app_service(appsrv)
  {
  }

  CliUiHandler::CliUiHandler(AppService& appsrv) : d(std::make_unique<CliUiHandlerImpl>(appsrv))
  {
  }
  CliUiHandler::~CliUiHandler()
  {
  }
  CliUiHandler::CliUiHandler(CliUiHandler&&) noexcept = default;
  CliUiHandler& CliUiHandler::operator=(CliUiHandler&&) noexcept = default;

  void CliUiHandler::start()
  {
  }

  void CliUiHandler::stop()
  {
  }

  OperationResult CliUiHandler::getResult()
  {
    OperationResult result;
    return result;
  }

} // namespace pendarlab::app::mavlink_hub