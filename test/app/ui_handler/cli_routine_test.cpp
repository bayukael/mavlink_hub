#include "app/shutdown_controller/ShutdownController.h"
#include "app/ui_handler/CliRoutine.h"

#include <gtest/gtest.h>
#include <stdexcept>

using pendarlab::app::mavlink_hub::CliRoutine;
using pendarlab::app::mavlink_hub::ShutdownController;

// CliUiHandler runs CliRoutine on its UI thread; these tests verify the
// shutdown-on-exit wiring that CliUiHandler relies on, without launching a real
// FTXUI interface (a fake routine body is used instead).

// The routine requests shutdown when its body exits normally (the user quits
// the UI), so waitForShutdownSignal() returns instead of App::run() hanging.
TEST(CliRoutineTest, RequestsShutdownOnNormalExit)
{
  ShutdownController shutdown_controller;
  CliRoutine routine(shutdown_controller, [] {});

  routine.run();

  // waitForShutdownSignal() must return immediately because the signal fired.
  shutdown_controller.waitForShutdownSignal();
}

// The shutdown signal still fires when the routine body ends by exception, so
// App::run() cannot hang even if the UI thread ends abnormally.
TEST(CliRoutineTest, RequestsShutdownOnExceptionInBody)
{
  ShutdownController shutdown_controller;
  CliRoutine routine(shutdown_controller, [] { throw std::runtime_error("boom"); });

  EXPECT_THROW(routine.run(), std::runtime_error);

  // waitForShutdownSignal() must return immediately (the guard fired during
  // stack unwinding) without deadlocking.
  shutdown_controller.waitForShutdownSignal();
}

// The body is executed and a single shutdown signal fires even when the body
// does not request shutdown on its own.
TEST(CliRoutineTest, RunsBodyAndSignalsShutdown)
{
  ShutdownController shutdown_controller;
  bool body_ran = false;
  CliRoutine routine(shutdown_controller, [&] { body_ran = true; });

  routine.run();

  EXPECT_EQ(true, body_ran);
  // Repeated waits are safe (the latch is latched); the first must return.
  shutdown_controller.waitForShutdownSignal();
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
