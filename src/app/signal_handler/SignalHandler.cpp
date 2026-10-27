#include "app/signal_handler/SignalHandler.h"

#include <algorithm>
#include <csignal>
#include <system_error>
#include <thread>

#if !defined(_WIN32)
#include <pthread.h>
#endif

namespace pendarlab::app::mavlink_hub
{
  namespace
  {
    // A private signal (never delivered to a handler) used only to wake the
    // sigwait() thread from stop(), so it can be joined even when shutdown was
    // triggered from a source other than a signal (e.g. the CLI quit path).
    constexpr int kWakeSignal = SIGUSR1;
  } // namespace

  struct SignalHandler::SignalHandlerImpl {
    SignalHandlerImpl(ShutdownController& shutdown_controller, std::vector<int> termination_signals)
        : shutdown_controller(shutdown_controller), termination_signals(std::move(termination_signals))
    {
    }

    ShutdownController& shutdown_controller;
    std::vector<int> termination_signals;

    // The full set of signals the sigwait() thread waits on: the termination
    // signals plus the private wake signal. Populated in start().
    sigset_t await_set;
    // The calling thread's mask before start() blocked the awaited signals;
    // restored by stop().
    sigset_t original_mask;
    bool started = false;
    std::thread sigwait_thread;

    // Build the union of termination signals plus the wake signal into await_set.
    void buildAwaitSet()
    {
      sigemptyset(&await_set);
      for (const int signal : termination_signals) {
        sigaddset(&await_set, signal);
      }
      sigaddset(&await_set, kWakeSignal);
    }
  };

  SignalHandler::SignalHandler(ShutdownController& shutdown_controller, std::vector<int> termination_signals)
      : d(std::make_unique<SignalHandlerImpl>(shutdown_controller, std::move(termination_signals)))
  {
  }
  SignalHandler::~SignalHandler()
  {
    stop();
  }
  SignalHandler::SignalHandler(SignalHandler&&) noexcept = default;
  SignalHandler& SignalHandler::operator=(SignalHandler&&) noexcept = default;

  void SignalHandler::start()
  {
    if (d->started) {
      return;
    }

    d->buildAwaitSet();

    // Block the awaited signals in the calling thread and capture the previous
    // mask so stop() can restore it. Every thread created after this point
    // inherits this blocked mask, keeping FTXUI's own handlers from ever
    // running on these signals.
    if (int rc = pthread_sigmask(SIG_BLOCK, &d->await_set, &d->original_mask); rc != 0) {
      throw std::system_error(rc, std::generic_category(), "SignalHandler: pthread_sigmask(SIG_BLOCK) failed");
    }

    try {
      d->sigwait_thread = std::thread([this] {
        // Runs inside the dedicated sigwait thread, which inherited the signals
        // blocked. sigwait() consumes pending signals; it returns with the
        // received signal number, so requestShutdown() (which locks a mutex) is
        // executed from a normal thread context, never from a signal handler.
        int received = 0;
        if (sigwait(&d->await_set, &received) == 0 && received != kWakeSignal) {
          d->shutdown_controller.requestShutdown();
        }
      });
    } catch (...) {
      // Restore the mask before propagating so the caller is not left with the
      // signals blocked if the thread could not be created.
      pthread_sigmask(SIG_SETMASK, &d->original_mask, nullptr);
      throw;
    }

    d->started = true;
  }

  void SignalHandler::stop()
  {
    if (!d->started) {
      return;
    }

    // Wake the sigwait() thread so it can be joined even when shutdown came from
    // a non-signal source. If the thread already exited (a termination signal
    // was received), pthread_kill returns ESRCH, which is fine.
    pthread_kill(d->sigwait_thread.native_handle(), kWakeSignal);
    if (d->sigwait_thread.joinable()) {
      d->sigwait_thread.join();
    }

    // Drain any signals that are still pending so restoring the mask cannot
    // deliver a leftover signal to a handler (which could kill the process).
    sigset_t pending;
    sigemptyset(&pending);
    sigpending(&pending);
    for (const int signal : d->termination_signals) {
      if (sigismember(&pending, signal)) {
        int discarded = 0;
        sigset_t one;
        sigemptyset(&one);
        sigaddset(&one, signal);
        sigwait(&one, &discarded);
      }
    }

    pthread_sigmask(SIG_SETMASK, &d->original_mask, nullptr);
    d->started = false;
  }

} // namespace pendarlab::app::mavlink_hub
