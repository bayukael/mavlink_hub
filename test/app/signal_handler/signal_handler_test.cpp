#include "app/shutdown_controller/ShutdownController.h"
#include "app/signal_handler/SignalHandler.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <thread>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>

using pendarlab::app::mavlink_hub::ShutdownController;
using pendarlab::app::mavlink_hub::SignalHandler;

namespace
{
  // How long a test is willing to wait for a waiter thread to be woken. The
  // wake path is deterministic (latch set + cv notify), so this is only a
  // generous upper bound to avoid spinning forever on a regression.
  constexpr std::chrono::milliseconds kWakeTimeout{ 2000 };

  // Raised a process-directed signal for the given signo and waits until a
  // waiter thread (parked in waitForShutdownSignal) returns, or fails.
  //
  // The signal must be blocked in the calling thread (and thus in every helper
  // thread created after SignalHandler::start()). A process-directed
  // kill(getpid(), sig) makes the signal pending for the process, so the
  // sigwait() thread consumes it and calls requestShutdown(), which wakes the
  // waiter.
  void verifySignalTriggersShutdown(int termination_signal)
  {
    ShutdownController controller;
    SignalHandler handler(controller, std::vector<int>{ SIGINT, SIGTERM });

    // start() blocks SIGINT/SIGTERM in this (test) thread and launches the
    // sigwait() thread. It must be OFF by default; here we explicitly enable it.
    handler.start();

    std::atomic<bool> entered{ false };
    std::atomic<bool> exited{ false };
    std::thread waiter([&] {
      entered.store(true, std::memory_order_release);
      controller.waitForShutdownSignal();
      exited.store(true, std::memory_order_release);
    });
    while (!entered.load(std::memory_order_acquire)) {
      std::this_thread::yield();
    }

    // Raise the termination signal from a helper thread. It is blocked in every
    // thread (inherited from the test thread after start()), so it stays pending
    // until the sigwait() thread consumes it -> requestShutdown().
    std::thread raiser([&] {
      ::kill(::getpid(), termination_signal);
    });
    raiser.join();

    // The waiter must be woken promptly.
    const auto deadline = std::chrono::steady_clock::now() + kWakeTimeout;
    while (!exited.load(std::memory_order_acquire) && std::chrono::steady_clock::now() < deadline) {
      std::this_thread::yield();
    }
    EXPECT_TRUE(exited.load(std::memory_order_acquire));

    // Clean up: wake + join the sigwait thread and restore the signal mask so
    // nothing stays blocked for subsequent tests/processes.
    handler.stop();
    waiter.join();
  }
} // namespace

// SIGTERM received by the process must drive the cooperative shutdown: it wakes
// waitForShutdownSignal() -> requestShutdown() runs -> the waiter returns.
TEST(SignalHandlerTest, SigmermTriggersCooperativeShutdown)
{
  verifySignalTriggersShutdown(SIGTERM);
}

// SIGINT received by the process must do the same.
TEST(SignalHandlerTest, SigmintTriggersCooperativeShutdown)
{
  verifySignalTriggersShutdown(SIGINT);
}

// Signal handling is strictly opt-in: constructing a SignalHandler and letting
// it be destroyed (or calling stop()) without ever calling start() must be a
// no-op and must not throw or alter the process. This documents the
// "OFF in unit-test / shared-lib contexts unless explicitly enabled" contract.
TEST(SignalHandlerTest, OffUnlessExplicitlyStarted)
{
  ShutdownController controller;
  {
    SignalHandler handler(controller, std::vector<int>{ SIGINT, SIGTERM });
    ASSERT_NO_THROW(handler.stop());  // not started -> no-op
  }
  SUCCEED();
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
