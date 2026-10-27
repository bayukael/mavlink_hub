#pragma once

#include "app/shutdown_controller/ShutdownController.h"

#include <memory>
#include <vector>

namespace pendarlab::app::mavlink_hub
{
  /// Owns SIGINT/SIGTERM so mavlink_hub can shut down gracefully instead of
  /// dying to a re-raised signal through FTXUI's handlers.
  ///
  /// FTXUI installs its own SIGINT/SIGTERM handlers when the UI starts
  /// (InstallSignalHandler -> RecordSignal). On such a signal it quits the UI
  /// loop, restores the terminal, then restores the original handler and
  /// re-raises the signal, killing the process (e.g. exit code 130 for SIGINT)
  /// without running the application's teardown. This class takes ownership of
  /// those signals *before* the UI starts so the cooperative shutdown path is
  /// used instead.
  ///
  /// Mechanism (async-signal-safe):
  ///  - start() blocks the handled signals in the *calling* thread, so every
  ///    thread created afterwards (including the FTXUI UI thread) inherits them
  ///    blocked. FTXUI may still install its own handlers, but because the
  ///    signals are blocked everywhere those handlers never run.
  ///  - start() then launches a dedicated thread that blocks in sigwait(). When
  ///    a handled termination signal arrives it is consumed by sigwait() (not by
  ///    any handler) and requestShutdown() is called from that normal thread
  ///    context - never from an actual signal handler - so it is safe to lock a
  ///    mutex. App::run()'s waitForShutdownSignal() then returns and the usual
  ///    teardown (cli_handler.stop() -> App::Exit()) restores the terminal.
  ///  - stop() wakes the waiting thread with a private signal, joins it, and
  ///    restores the calling thread's original signal mask.
  ///
  /// The wiring is opt-in: it must be started explicitly by the caller. The
  /// application path (App::run) starts it; unit-test and shared-library
  /// contexts leave it off unless they explicitly start it, so it never hijacks
  /// signals in the test suite.
  class SignalHandler
  {
  public:
    /// @param shutdown_controller the controller to request shutdown on when a
    ///        termination signal is received. Must outlive this handler.
    /// @param termination_signals the signal numbers to own (e.g. {SIGINT,
    ///        SIGTERM}). Each is blocked + awaited by start().
    SignalHandler(ShutdownController& shutdown_controller, std::vector<int> termination_signals);
    ~SignalHandler();
    SignalHandler(SignalHandler&&) noexcept;
    SignalHandler& operator=(SignalHandler&&) noexcept;

    /// Block the handled signals in the calling thread and launch the sigwait()
    /// thread. No-op if already started. Throws std::system_error on failure.
    void start();

    /// Wake and join the sigwait() thread, then restore the calling thread's
    /// original signal mask. No-op if not started or already stopped.
    void stop();

  private:
    struct SignalHandlerImpl;
    std::unique_ptr<SignalHandlerImpl> d;
  };
} // namespace pendarlab::app::mavlink_hub
