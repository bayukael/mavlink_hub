#pragma once

#include "app/shutdown_controller/ShutdownController.h"

#include <functional>

namespace pendarlab::app::mavlink_hub
{
  /// Runs an interactive UI-generating routine while guaranteeing that shutdown is
  /// signaled when the routine exits, on every exit path.
  ///
  /// CliUiHandler uses this to run its FTXUI UI-generating function on a background thread:
  /// when the routine finishes (user quits the UI) or throws (the UI thread ends abnormally),
  /// it fires requestShutdown() on the attached ShutdownController so a caller waiting on
  /// waitForShutdownSignal() (e.g. App::run()) returns instead of hanging.
  ///
  /// It is a standalone, terminal-free class so the shutdown-on-exit wiring can be unit tested
  /// without launching an actual FTXUI interface; tests pass a fake routine body.
  class CliRoutine
  {
  public:
    /// @param shutdown_controller the controller to request shutdown on when the routine ends.
    ///        Must outlive this routine.
    /// @param routine_body the function that generates/renders the UI on the calling thread.
    CliRoutine(ShutdownController& shutdown_controller, std::function<void()> routine_body);

    /// Fires requestShutdown() on the attached controller. CliRoutine is constructed and
    /// destroyed on the same thread that runs the routine, so the destructor runs on that
    /// thread when the routine ends — including during exception unwinding — keeping the
    /// shutdown signal guaranteed on every exit path.
    ~CliRoutine();

    /// Execute routine_body on the calling thread. On exit (normal return, early return, or an
    /// exception unwinding the stack) the destructor requests shutdown on the attached
    /// controller, so waitForShutdownSignal() can never hang.
    void run();

  private:
    ShutdownController& shutdown_controller;
    std::function<void()> routine_body;
  };
} // namespace pendarlab::app::mavlink_hub
