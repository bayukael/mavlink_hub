#include "app/ui_handler/CliRoutine.h"

#include <utility>

namespace pendarlab::app::mavlink_hub
{
  // RAII guard that fires requestShutdown() (latched/idempotent) when the scope it was
  // created in is exited, whether by normal return, early return, or an exception unwinding
  // the stack. This guarantees the shutdown signal always fires when the routine ends, so
  // App::run() never hangs on waitForShutdownSignal() even if the UI thread ends abnormally.
  class RoutineShutdownGuard
  {
  public:
    explicit RoutineShutdownGuard(ShutdownController& controller) : shutdown_controller(controller) {}
    ~RoutineShutdownGuard() { shutdown_controller.requestShutdown(); }

    RoutineShutdownGuard(const RoutineShutdownGuard&) = delete;
    RoutineShutdownGuard& operator=(const RoutineShutdownGuard&) = delete;

  private:
    ShutdownController& shutdown_controller;
  };

  CliRoutine::CliRoutine(ShutdownController& shutdown_controller, std::function<void()> routine_body)
      : shutdown_controller(shutdown_controller), routine_body(std::move(routine_body))
  {
  }

  void CliRoutine::run()
  {
    RoutineShutdownGuard guard(shutdown_controller);
    routine_body();
  }
} // namespace pendarlab::app::mavlink_hub
