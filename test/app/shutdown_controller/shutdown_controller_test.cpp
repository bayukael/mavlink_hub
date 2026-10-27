#include "app/shutdown_controller/ShutdownController.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

using pendarlab::app::mavlink_hub::ShutdownController;

namespace
{
  // How long a test is willing to observe a thread that is *supposed* to still
  // be parked. This never is the sole synchronization -- every blocking test
  // below uses a real handshake (an atomic entered-flag) so it is
  // deterministic -- but it bounds how long we wait before declaring that a
  // waiter did not return too early. Kept generous so it is not fragile under
  // scheduler load.
  constexpr std::chrono::milliseconds kStillBlockedGrace{ 200 };

  // A thread that parks in waitForShutdownSignal and records when it returns.
  // Used whenever the point of a test is "wait returns with a single wake-up".
  struct Waiter
  {
    std::thread thread;
    std::atomic<bool> entered{ false };
    std::atomic<bool> exited{ false };

    explicit Waiter(ShutdownController& controller)
    {
      thread = std::thread([this, &controller]() {
        entered.store(true, std::memory_order_release);
        controller.waitForShutdownSignal();
        exited.store(true, std::memory_order_release);
      });
    }

    // Bounded busy-wait until the thread has actually parked in waitForShutdownSignal.
    void waitUntilEntered()
    {
      while (!entered.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }
    }

    // Block until the thread returned from waitForShutdownSignal, then join it.
    void waitUntilExited()
    {
      while (!exited.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }
      thread.join();
      thread = std::thread();
    }

    ~Waiter()
    {
      if (thread.joinable()) {
        thread.join();
      }
    }

    Waiter(const Waiter&) = delete;
    Waiter& operator=(const Waiter&) = delete;
    Waiter(Waiter&&) = delete;
    Waiter& operator=(Waiter&&) = delete;
  };
} // namespace

class ShutdownControllerTest : public testing::Test
{
protected:
  // A fresh controller per test so no state leaks between cases.
  ShutdownController controller;
};

TEST_F(ShutdownControllerTest, WaitReturnsImmediatelyAfterRequestAlreadyHappened)
{
  controller.requestShutdown();

  // Must not block: the flag was already latched before we ever waited.
  controller.waitForShutdownSignal();
}

TEST_F(ShutdownControllerTest, WaitUnblocksWhenRequestIsCalled)
{
  Waiter waiter(controller);
  waiter.waitUntilEntered();

  controller.requestShutdown();

  waiter.waitUntilExited();
  EXPECT_TRUE(waiter.exited.load(std::memory_order_acquire));
}

TEST_F(ShutdownControllerTest, WaitBlocksUntilRequestComesFromAnotherThread)
{
  Waiter waiter(controller);
  waiter.waitUntilEntered();

  // While no request is issued the waiter must stay parked. We observe the
  // negative case only on top of the positive handshake below, so this cannot
  // miss a real wake-up; it only confirms the waiter did not return early.
  std::this_thread::sleep_for(kStillBlockedGrace);
  EXPECT_FALSE(waiter.exited.load(std::memory_order_acquire));

  controller.requestShutdown();
  waiter.waitUntilExited();
  EXPECT_TRUE(waiter.exited.load(std::memory_order_acquire));
}

TEST_F(ShutdownControllerTest, DuplicateRequestsAreHarmless)
{
  // Multiple requests in a row must not deadlock, crash, or corrupt the latch.
  controller.requestShutdown();
  controller.requestShutdown();
  controller.requestShutdown();

  controller.waitForShutdownSignal();
  SUCCEED();
}

TEST_F(ShutdownControllerTest, DuplicateRequestsAfterSignalAreHarmless)
{
  // Exercise the "after-the-fact" path: request, observe it unblocked a
  // waiter, then keep requesting -- nothing should double-notify or crash.
  Waiter waiter(controller);
  waiter.waitUntilEntered();

  controller.requestShutdown();
  waiter.waitUntilExited();

  controller.requestShutdown();
  controller.requestShutdown();

  SUCCEED();
}

TEST_F(ShutdownControllerTest, AllConcurrentWaitersAreWokenOnRequest)
{
  constexpr int kWaiterCount = 8;
  std::vector<std::unique_ptr<Waiter>> waiters;
  waiters.reserve(kWaiterCount);
  for (int i = 0; i < kWaiterCount; ++i) {
    waiters.push_back(std::make_unique<Waiter>(controller));
  }
  for (auto& w : waiters) {
    w->waitUntilEntered();
  }

  controller.requestShutdown();

  for (auto& w : waiters) {
    w->waitUntilExited();
    EXPECT_TRUE(w->exited.load(std::memory_order_acquire));
  }
}

TEST_F(ShutdownControllerTest, ConcurrentRequestsAreThreadSafe)
{
  // Many threads race to trigger the latch at roughly the same time. The latch
  // must survive the race: no deadlock, no crash, and waiters wake regardless
  // of which request wins.
  constexpr int kProducerCount = 8;
  std::atomic<bool> go{ false };

  std::vector<std::thread> producers;
  producers.reserve(kProducerCount);
  for (int i = 0; i < kProducerCount; ++i) {
    producers.emplace_back([this, &go]() {
      while (!go.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }
      controller.requestShutdown();
    });
  }

  // Give every producer a chance to reach the gate before releasing them
  // together, so the requests genuinely overlap.
  std::this_thread::sleep_for(kStillBlockedGrace);
  go.store(true, std::memory_order_release);

  for (auto& p : producers) {
    p.join();
  }
  producers.clear();

  // Whatever the interleaving, the resulting state is a single latched
  // controller: a subsequent wait must return immediately.
  controller.waitForShutdownSignal();
}

TEST_F(ShutdownControllerTest, ConcurrentRequestsWakeAllWaiters)
{
  // Mix concurrent producers with concurrent waiters: the race must not
  // deadlock and every waiter must eventually be woken.
  constexpr int kProducerCount = 4;
  constexpr int kWaiterCount = 4;

  std::atomic<bool> go{ false };

  std::vector<std::thread> producers;
  producers.reserve(kProducerCount);
  for (int i = 0; i < kProducerCount; ++i) {
    producers.emplace_back([this, &go]() {
      while (!go.load(std::memory_order_acquire)) {
        std::this_thread::yield();
      }
      controller.requestShutdown();
    });
  }

  std::vector<std::unique_ptr<Waiter>> waiters;
  waiters.reserve(kWaiterCount);
  for (int i = 0; i < kWaiterCount; ++i) {
    waiters.push_back(std::make_unique<Waiter>(controller));
  }
  for (auto& w : waiters) {
    w->waitUntilEntered();
  }

  go.store(true, std::memory_order_release);

  for (auto& p : producers) {
    p.join();
  }
  for (auto& w : waiters) {
    w->waitUntilExited();
    EXPECT_TRUE(w->exited.load(std::memory_order_acquire));
  }
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
