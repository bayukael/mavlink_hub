#include "app/shutdown_controller/ShutdownController.h"
#include "app/ui_handler/CliRoutine.h"

#include <gtest/gtest.h>
#include <stdexcept>

using pendarlab::app::mavlink_hub::CliRoutine;
using pendarlab::app::mavlink_hub::ShutdownController;

// CliUiHandler runs CliRoutine on its UI thread; these tests verify the
// shutdown-on-exit wiring that CliUiHandler relies on, without launching a real
// FTXUI interface (a fake routine body is used instead).
//
// CliRoutine fires requestShutdown() from its destructor, which runs when the
// routine ends (normal or via exception unwinding) on the thread that owns it.
// Each test therefore waits for the shutdown signal only after the routine has
// been destroyed.

// The routine requests shutdown when its body exits normally (the user quits
// the UI), so waitForShutdownSignal() returns instead of App::run() hanging.
TEST(CliRoutineTest, RequestsShutdownOnNormalExit)
{
  ShutdownController shutdown_controller;
  {
    CliRoutine routine(shutdown_controller, [] {});
    routine.run();
  } // destructor fires requestShutdown() here

  // waitForShutdownSignal() must return immediately because the signal fired.
  shutdown_controller.waitForShutdownSignal();
}

// The shutdown signal still fires when the routine body ends by exception, so
// App::run() cannot hang even if the UI thread ends abnormally.
TEST(CliRoutineTest, RequestsShutdownOnExceptionInBody)
{
  ShutdownController shutdown_controller;
  {
    CliRoutine routine(shutdown_controller, [] { throw std::runtime_error("boom"); });

    EXPECT_THROW(routine.run(), std::runtime_error);
  } // destructor fires requestShutdown() during normal scope exit

  // waitForShutdownSignal() must return immediately without deadlocking.
  shutdown_controller.waitForShutdownSignal();
}

// The body is executed and a shutdown signal fires even when the body does not
// request shutdown on its own.
TEST(CliRoutineTest, RunsBodyAndSignalsShutdown)
{
  ShutdownController shutdown_controller;
  bool body_ran = false;
  {
    CliRoutine routine(shutdown_controller, [&] { body_ran = true; });
    routine.run();
    EXPECT_EQ(true, body_ran);
  }

  // Repeated waits are safe (the flag is latched); the first must return.
  shutdown_controller.waitForShutdownSignal();
}

// Destruction fires the signal even when run() was never called (e.g. the UI
// thread ends before the routine starts) — the signal is tied to the routine
// object's lifetime, not to run() being entered.
TEST(CliRoutineTest, RequestsShutdownOnDestructionWithoutRun)
{
  ShutdownController shutdown_controller;
  {
    CliRoutine routine(shutdown_controller, [] {});
  } // destructor fires requestShutdown() here

  shutdown_controller.waitForShutdownSignal();
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
