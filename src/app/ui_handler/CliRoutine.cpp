#include "app/ui_handler/CliRoutine.h"

#include <utility>

namespace pendarlab::app::mavlink_hub
{
  CliRoutine::CliRoutine(ShutdownController& shutdown_controller, std::function<void()> routine_body)
      : shutdown_controller(shutdown_controller), routine_body(std::move(routine_body))
  {
  }

  CliRoutine::~CliRoutine()
  {
    // Fires requestShutdown() (latched/idempotent) when this routine is destroyed. CliRoutine
    // is constructed and destroyed on the same thread that runs the routine, so this runs on
    // that thread when the routine ends — including during exception unwinding — keeping the
    // shutdown signal guaranteed on every exit path, so App::run() never hangs on
    // waitForShutdownSignal() even if the UI thread ends abnormally.
    shutdown_controller.requestShutdown();
  }

  void CliRoutine::run()
  {
    routine_body();
  }
} // namespace pendarlab::app::mavlink_hub
