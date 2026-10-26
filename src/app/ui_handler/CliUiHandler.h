#pragma once

#include "app/app_service/IAppService.h"
#include "app/shutdown_controller/ShutdownController.h"
#include "common/types/OperationResult.h"

#include <functional>
#include <memory>

namespace pendarlab::app::mavlink_hub
{
  /// Full-screen FTXUI-based view that renders the CliUiController.
  ///
  /// Owns a CliUiController and runs a UI thread that blocks on the FTXUI event loop. It
  /// binds the controller's state to the rendered component tree and wires up the redraw
  /// callback so completed commands refresh the UI automatically.
  ///
  /// The UI thread also drives the ShutdownController: when the UI thread function exits
  /// (the user quits the UI, or the thread ends abnormally), it requests shutdown so a
  /// caller waiting on waitForShutdownSignal() (e.g. App::run()) returns instead of
  /// hanging. The signal is fired on every exit path, not just the normal quit path.
  class CliUiHandler
  {
  public:
    /// @param appsrv the service backing the UI's controller. Must outlive this handler.
    /// @param shutdown_controller the controller to request shutdown on when the UI thread
    ///        ends. Must outlive this handler.
    CliUiHandler(IAppService& appsrv, ShutdownController& shutdown_controller);
    ~CliUiHandler();
    CliUiHandler(CliUiHandler&&) noexcept;
    CliUiHandler& operator=(CliUiHandler&&) noexcept;

    /// Launch the UI on a background thread. No-op if already running.
    void start();

    /// Tear down the UI, waiting for any in-flight command to finish first. No-op if not
    /// running. Called automatically by the destructor.
    void stop();

    /// Run \p body on the calling thread wrapped in the same crash-safe shutdown guard the
    /// UI thread uses: when \p body returns or throws, requestShutdown() is fired on the
    /// attached ShutdownController, so waitForShutdownSignal() can never hang. Exposed so the
    /// non-interactive exit wiring can be unit tested without a terminal.
    void runWithShutdownGuard(std::function<void()> body);

    /// @return the handler's result after a UI session (currently always a default/empty
    ///         OperationResult).
    OperationResult getResult();

  private:
    struct CliUiHandlerImpl;
    std::unique_ptr<CliUiHandlerImpl> d;
  };

} // namespace pendarlab::app::mavlink_hub
