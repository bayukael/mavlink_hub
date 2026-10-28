#include "app/shutdown_controller/ShutdownController.h"
#include "app/ui_handler/CliUiHandler.h"
#include "mock_app_service/MockAppService.h"

#include <chrono>

int main()
{
  using pendarlab::app::mavlink_hub::CliUiHandler;
  using pendarlab::app::mavlink_hub::ShutdownController;
  using pendarlab::app::mavlink_hub::test::MockAppService;

  MockAppService app_service;
  app_service.slow_command = pendarlab::app::mavlink_hub::UserCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = std::chrono::seconds(5);
  ShutdownController shutdown_controller;
  CliUiHandler cli_handler(app_service, shutdown_controller);

  cli_handler.start();

  // Block until the UI thread exits (user quits); the handler then requests
  // shutdown, so this returns and the preview tears down cleanly.
  shutdown_controller.waitForShutdownSignal();

  cli_handler.stop();
  return 0;
}
