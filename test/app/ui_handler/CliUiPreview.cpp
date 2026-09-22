#include "app/ui_handler/CliUiHandler.h"
#include "mock_app_service/MockAppService.h"

#include <chrono>
#include <thread>

int main()
{
  using pendarlab::app::mavlink_hub::CliUiHandler;
  using pendarlab::app::mavlink_hub::test::MockAppService;

  MockAppService app_service;
  app_service.slow_command = pendarlab::app::mavlink_hub::UserCommandType::APPLY_CURRENT_PLAN;
  app_service.slow_delay = std::chrono::seconds(5);
  CliUiHandler cli_handler(app_service);

  cli_handler.start();

  // Keep the preview running until the UI is closed or the process is killed.
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  cli_handler.stop();
  return 0;
}
