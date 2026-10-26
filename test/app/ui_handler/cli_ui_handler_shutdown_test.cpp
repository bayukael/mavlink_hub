#include "app/shutdown_controller/ShutdownController.h"
#include "app/ui_handler/CliUiHandler.h"
#include "mock_app_service/MockAppService.h"

#include <gtest/gtest.h>
#include <functional>
#include <stdexcept>
#include <utility>

using pendarlab::app::mavlink_hub::CliUiHandler;
using pendarlab::app::mavlink_hub::ShutdownController;
using pendarlab::app::mavlink_hub::test::MockAppService;

// The CliUiHandler signals shutdown when its thread body exits normally (the
// user quits the UI), so App::run() returns instead of hanging.
TEST(CliUiHandlerShutdownTest, RequestsShutdownOnNormalExit)
{
  ShutdownController shutdown_controller;
  MockAppService app_service;
  CliUiHandler handler(app_service, shutdown_controller);

  bool body_ran = false;
  handler.runWithShutdownGuard([&] { body_ran = true; });

  EXPECT_TRUE(body_ran);
  // waitForShutdownSignal must return immediately because the signal fired.
  shutdown_controller.waitForShutdownSignal();
}

// The shutdown signal still fires when the thread body ends by exception, so
// App::run() cannot hang even if the UI thread ends abnormally.
TEST(CliUiHandlerShutdownTest, RequestsShutdownOnExceptionInBody)
{
  ShutdownController shutdown_controller;
  MockAppService app_service;
  CliUiHandler handler(app_service, shutdown_controller);

  EXPECT_THROW(handler.runWithShutdownGuard([] { throw std::runtime_error("boom"); }),
               std::runtime_error);

  // waitForShutdownSignal must return immediately (the guard fired during stack
  // unwinding) without deadlocking.
  shutdown_controller.waitForShutdownSignal();
}

// A body that never signals on its own still causes shutdown once it exits.
TEST(CliUiHandlerShutdownTest, SignalFiresExactlyOnceOnNormalExit)
{
  ShutdownController shutdown_controller;
  MockAppService app_service;
  CliUiHandler handler(app_service, shutdown_controller);

  handler.runWithShutdownGuard([] {});

  // Repeated wait calls are safe (the latch is latched); the first must return.
  shutdown_controller.waitForShutdownSignal();
  shutdown_controller.waitForShutdownSignal();
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
